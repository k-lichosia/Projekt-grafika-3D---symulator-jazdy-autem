#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include "shaderprogram.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <cctype> // Potrzebne do obs³ugi ma³ych/wielkich liter

extern ShaderProgram* sp;

// Struktura przechowuj¹ca kolor z pliku .mtl
struct Material {
	float r = 0.8f, g = 0.8f, b = 0.8f; // Domyœlnie szary
};

// Struktura przechowuj¹ca fragment modelu (np. tylko opony, tylko szyby)
struct MeshPart {
	Material mat;
	std::string matName; 
	std::vector<float> vertices;
	std::vector<float> normals;
	GLuint vboVertices = 0;
	GLuint vboNormals = 0;
	GLuint vao = 0;
};

struct ObjModel {
	std::vector<MeshPart> parts;
	std::map<std::string, Material> materials;

	// Funkcja pomocnicza czytaj¹ca plik .mtl
	void loadMtl(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Ostrzezenie: Nie udalo sie znalezc pliku z kolorami: " << filename << std::endl;
			return;
		}

		std::string line, currentMatName;
		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::string type;
			ss >> type;

			if (type == "newmtl") {
				ss >> currentMatName;
			}
			else if (type == "Kd" && !currentMatName.empty()) {
				ss >> materials[currentMatName].r >> materials[currentMatName].g >> materials[currentMatName].b;
			}
		}
		std::cout << "Wczytano kolory z pliku " << filename << std::endl;
	}

	bool load(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Nie udalo sie otworzyc pliku modelu: " << filename << std::endl;
			return false;
		}

		std::vector<float> raw_vertices;
		std::vector<float> raw_normals;

		parts.clear();
		materials.clear();

		std::string line;
		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::string type;
			ss >> type;

			if (type == "mtllib") {
				std::string mtlFilename;
				ss >> mtlFilename;
				loadMtl(mtlFilename);
			}
			else if (type == "usemtl") {
				std::string matName;
				ss >> matName;

				parts.push_back(MeshPart());
				parts.back().matName = matName; // Zapisujemy nazwê materia³u dla tej czêœci

				if (materials.count(matName)) {
					parts.back().mat = materials[matName];
				}
			}
			else if (type == "v") {
				float x, y, z;
				ss >> x >> y >> z;
				raw_vertices.push_back(x); raw_vertices.push_back(y); raw_vertices.push_back(z);
			}
			else if (type == "vn") {
				float x, y, z;
				ss >> x >> y >> z;
				raw_normals.push_back(x); raw_normals.push_back(y); raw_normals.push_back(z);
			}
			else if (type == "f") {
				if (parts.empty()) parts.push_back(MeshPart());

				for (int i = 0; i < 3; ++i) {
					std::string vertexStr;
					ss >> vertexStr;
					int vIdx = -1, tIdx = -1, nIdx = -1;

					if (vertexStr.find("//") != std::string::npos) {
						sscanf_s(vertexStr.c_str(), "%d//%d", &vIdx, &nIdx);
					}
					else if (std::count(vertexStr.begin(), vertexStr.end(), '/') == 2) {
						sscanf_s(vertexStr.c_str(), "%d/%d/%d", &vIdx, &tIdx, &nIdx);
					}
					else if (std::count(vertexStr.begin(), vertexStr.end(), '/') == 1) {
						sscanf_s(vertexStr.c_str(), "%d/%d", &vIdx, &tIdx);
					}
					else {
						sscanf_s(vertexStr.c_str(), "%d", &vIdx);
					}

					if (vIdx > 0) {
						int base = (vIdx - 1) * 3;
						parts.back().vertices.push_back(raw_vertices[base]);
						parts.back().vertices.push_back(raw_vertices[base + 1]);
						parts.back().vertices.push_back(raw_vertices[base + 2]);
					}
					if (nIdx > 0 && !raw_normals.empty()) {
						int base = (nIdx - 1) * 3;
						parts.back().normals.push_back(raw_normals[base]);
						parts.back().normals.push_back(raw_normals[base + 1]);
						parts.back().normals.push_back(raw_normals[base + 2]);
					}
				}
			}
		}

		// --- WYSYLANIE MODELU DO KARTY GRAFICZNEJ (VBO) ---
		for (auto& part : parts) {
			if (!part.vertices.empty()) {
				glGenBuffers(1, &part.vboVertices);
				glBindBuffer(GL_ARRAY_BUFFER, part.vboVertices);
				glBufferData(GL_ARRAY_BUFFER, part.vertices.size() * sizeof(float), part.vertices.data(), GL_STATIC_DRAW);
			}
			if (!part.normals.empty()) {
				glGenBuffers(1, &part.vboNormals);
				glBindBuffer(GL_ARRAY_BUFFER, part.vboNormals);
				glBufferData(GL_ARRAY_BUFFER, part.normals.size() * sizeof(float), part.normals.data(), GL_STATIC_DRAW);
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0); // Odpinamy bufor

		std::cout << "Model " << filename << " gotowy!" << std::endl;
		return true;
	}

	void draw(float customR = -1.0f, float customG = -1.0f, float customB = -1.0f) {
		GLint vertexLoc = sp->a("vertex");
		GLint normalLoc = sp->a("normal");
		GLint colorLoc = sp->u("objectColor");

		// UWAGA: Usunieto 'const', bo bedziemy zapisywac wygenerowane VAO do struktury
		for (auto& part : parts) {
			if (part.vertices.empty()) continue;

			bool zmienKolor = false;

			if (customR >= 0.0f && customG >= 0.0f && customB >= 0.0f) {
				std::string nameLower = part.matName;
				std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

				if (nameLower.find("body") != std::string::npos || nameLower.find("paint") != std::string::npos ||
					nameLower.find("karoseria") != std::string::npos || nameLower.find("car") != std::string::npos ||
					nameLower.find("exterior") != std::string::npos || nameLower.find("material") != std::string::npos) {

					if (nameLower.find("wheel") == std::string::npos && nameLower.find("tire") == std::string::npos &&
						nameLower.find("glass") == std::string::npos && nameLower.find("window") == std::string::npos &&
						nameLower.find("opon") == std::string::npos && nameLower.find("szyb") == std::string::npos) {
						zmienKolor = true;
					}
				}
			}

			float r = zmienKolor ? customR : part.mat.r;
			float g = zmienKolor ? customG : part.mat.g;
			float b = zmienKolor ? customB : part.mat.b;

			glUniform3f(colorLoc, r, g, b);

			// ==========================================
			// TWORZENIE I AKTYWACJA VAO DLA WERSJI 330
			// ==========================================
			if (part.vao == 0) {
				glGenVertexArrays(1, &part.vao); // Generujemy tylko raz
			}
			glBindVertexArray(part.vao); // Aktywujemy VAO

			if (part.vboVertices != 0 && vertexLoc != -1) {
				glEnableVertexAttribArray(vertexLoc);
				glBindBuffer(GL_ARRAY_BUFFER, part.vboVertices);
				glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			}

			if (part.vboNormals != 0 && normalLoc != -1) {
				glEnableVertexAttribArray(normalLoc);
				glBindBuffer(GL_ARRAY_BUFFER, part.vboNormals);
				glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			}

			// Rysowanie! Karta graficzna widzi aktywne VAO, wiec pozwala na rysowanie.
			glDrawArrays(GL_TRIANGLES, 0, part.vertices.size() / 3);

			// Sprzatanie po narysowaniu czesci modelu
			if (normalLoc != -1) glDisableVertexAttribArray(normalLoc);
			if (vertexLoc != -1) glDisableVertexAttribArray(vertexLoc);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0); // Odpinamy VAO
		}
	}
};