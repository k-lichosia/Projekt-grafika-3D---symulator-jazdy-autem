#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <GL/glew.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <cctype> // Potrzebne do obs³ugi ma³ych/wielkich liter

// Struktura przechowuj¹ca kolor z pliku .mtl
struct Material {
	float r = 0.8f, g = 0.8f, b = 0.8f; // Domyœlnie szary
};

// Struktura przechowuj¹ca fragment modelu (np. tylko opony, tylko szyby)
struct MeshPart {
	Material mat;
	std::string matName; // PAMIÊTAMY NAZWÊ: teraz zapisujemy jak nazywa siê ta czêœæ!
	std::vector<float> vertices;
	std::vector<float> normals;
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
		std::cout << "Model " << filename << " gotowy!" << std::endl;
		return true;
	}

	void draw(float customR = -1.0f, float customG = -1.0f, float customB = -1.0f) {
		for (const auto& part : parts) {
			if (part.vertices.empty()) continue;

			bool zmienKolor = false;

			// Jeœli podano losowy kolor, sprawdzamy czy ta czêœæ to karoseria
			if (customR >= 0.0f && customG >= 0.0f && customB >= 0.0f) {
				// Zamieniamy nazwê na ma³e litery, ¿eby wielkoœæ znaków nie mia³a znaczenia
				std::string nameLower = part.matName;
				std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

				// Szukamy s³ów kluczowych odpowiadaj¹cych za lakier/karoseriê
				if (nameLower.find("body") != std::string::npos ||
					nameLower.find("paint") != std::string::npos ||
					nameLower.find("karoseria") != std::string::npos ||
					nameLower.find("car") != std::string::npos ||
					nameLower.find("exterior") != std::string::npos ||
					nameLower.find("material") != std::string::npos) { // Czêsta nazwa z Blendera

					// ZABEZPIECZENIE: Jeœli nazwa zawiera "wheel", "glass" itd., to jej NIE zmieniamy
					if (nameLower.find("wheel") == std::string::npos &&
						nameLower.find("tire") == std::string::npos &&
						nameLower.find("glass") == std::string::npos &&
						nameLower.find("window") == std::string::npos &&
						nameLower.find("opon") == std::string::npos &&
						nameLower.find("szyb") == std::string::npos) {

						zmienKolor = true;
					}
				}
			}

			// Malujemy odpowiednim kolorem
			if (zmienKolor) {
				glColor3f(customR, customG, customB); // Losowy lakier karoserii
			}
			else {
				glColor3f(part.mat.r, part.mat.g, part.mat.b); // Oryginalny kolor ko³a/szyby z MTL
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, part.vertices.data());

			if (!part.normals.empty()) {
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, 0, part.normals.data());
			}

			glDrawArrays(GL_TRIANGLES, 0, part.vertices.size() / 3);

			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}
};