#define _CRT_SECURE_NO_WARNINGS
#pragma once

// =======================================================================
// --- BIBLIOTEKI ---
// =======================================================================
#include "shaderprogram.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <cctype> 

extern ShaderProgram* sp;

// =======================================================================
// --- STRUKTURY DANYCH MODELU 3D ---
// =======================================================================

// Struktura przechowujaca bazowy kolor obiektu
struct Material {
    float r = 0.8f, g = 0.8f, b = 0.8f;
};

// Struktura przechowujaca pojedynczy fragment modelu (posiadajacy jeden material)
struct MeshPart {
    Material mat;
    std::string matName;

    std::vector<float> vertices;
    std::vector<float> normals;

    // Identyfikatory buforow na karcie graficznej
    GLuint vboVertices = 0;
    GLuint vboNormals = 0;
    GLuint vao = 0;
};

// Glowna klasa zarzadzajaca calym modelem wczytanym z pliku OBJ
struct ObjModel {
    std::vector<MeshPart> parts;
    std::map<std::string, Material> materials;

    // =======================================================================
    // --- LADOWANIE MATERIALOW (.MTL) ---
    // =======================================================================
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
                ss >> currentMatName; // Nowy material
            }
            else if (type == "Kd" && !currentMatName.empty()) {
                // Kd oznacza kolor podstawowy (Diffuse) - pobieramy wartosci RGB
                ss >> materials[currentMatName].r >> materials[currentMatName].g >> materials[currentMatName].b;
            }
        }
        std::cout << "Wczytano kolory z pliku " << filename << std::endl;
    }

    // =======================================================================
    // --- LADOWANIE GEOMETRII (.OBJ) ---
    // =======================================================================
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
                // Powiazany plik z materialami
                std::string mtlFilename;
                ss >> mtlFilename;
                loadMtl(mtlFilename);
            }
            else if (type == "usemtl") {
                // Rozpoczecie nowej czesci siatki (MeshPart) dla danego materialu
                std::string matName;
                ss >> matName;

                parts.push_back(MeshPart());
                parts.back().matName = matName;

                if (materials.count(matName)) {
                    parts.back().mat = materials[matName];
                }
            }
            else if (type == "v") {
                // Wierzcholek (Vertex)
                float x, y, z;
                ss >> x >> y >> z;
                raw_vertices.push_back(x); raw_vertices.push_back(y); raw_vertices.push_back(z);
            }
            else if (type == "vn") {
                // Wektor normalny (Normal)
                float x, y, z;
                ss >> x >> y >> z;
                raw_normals.push_back(x); raw_normals.push_back(y); raw_normals.push_back(z);
            }
            else if (type == "f") {
                // Sciana (Face) - skladanie wierzcholkow w trojkaty
                if (parts.empty()) parts.push_back(MeshPart());

                for (int i = 0; i < 3; ++i) {
                    std::string vertexStr;
                    ss >> vertexStr;
                    int vIdx = -1, tIdx = -1, nIdx = -1;

                    // Obsluga roznych formatow zapisu scian (v, v//n, v/t/n, v/t)
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

                    // Zapisywanie rozkodowanych wierzcholkow do odpowiedniej czesci
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

        // =======================================================================
        // --- TWORZENIE BUFOROW VBO NA KARCIE GRAFICZNEJ ---
        // =======================================================================
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
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        std::cout << "Model " << filename << " gotowy!" << std::endl;
        return true;
    }

    // =======================================================================
    // --- RYSOWANIE MODELU Z OPCJONALNA PODMIANA KOLORU KAROSERII ---
    // =======================================================================
    void draw(float customR = -1.0f, float customG = -1.0f, float customB = -1.0f) {
        GLint vertexLoc = sp->a("vertex");
        GLint normalLoc = sp->a("normal");
        GLint colorLoc = sp->u("objectColor");

        for (auto& part : parts) {
            if (part.vertices.empty()) continue;

            bool zmienKolor = false;

            // Logika podmieniajaca tylko kolor glownej bryly aut (omijajac szyby i kola)
            if (customR >= 0.0f && customG >= 0.0f && customB >= 0.0f) {
                std::string nameLower = part.matName;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

                // Szukamy slow kluczowych dla karoserii
                if (nameLower.find("body") != std::string::npos || nameLower.find("paint") != std::string::npos ||
                    nameLower.find("karoseria") != std::string::npos || nameLower.find("car") != std::string::npos ||
                    nameLower.find("exterior") != std::string::npos || nameLower.find("material") != std::string::npos) {

                    // Upewniamy sie, ze to NIE sa opony ani szyby
                    if (nameLower.find("wheel") == std::string::npos && nameLower.find("tire") == std::string::npos &&
                        nameLower.find("glass") == std::string::npos && nameLower.find("window") == std::string::npos &&
                        nameLower.find("opon") == std::string::npos && nameLower.find("szyb") == std::string::npos) {
                        zmienKolor = true;
                    }
                }
            }

            // Ustawienie odpowiedniego koloru do shadera
            float r = zmienKolor ? customR : part.mat.r;
            float g = zmienKolor ? customG : part.mat.g;
            float b = zmienKolor ? customB : part.mat.b;

            glUniform3f(colorLoc, r, g, b);

            // Inicjalizacja VAO, jesli jeszcze nie istnieje
            if (part.vao == 0) {
                glGenVertexArrays(1, &part.vao);
            }
            glBindVertexArray(part.vao);

            // Podlaczenie buforow z wierzcholkami i wektorami normalnymi
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

            // Faktyczne rysowanie trojkatow dla danej czesci
            glDrawArrays(GL_TRIANGLES, 0, part.vertices.size() / 3);

            // Sprzatanie po rysowaniu
            if (normalLoc != -1) glDisableVertexAttribArray(normalLoc);
            if (vertexLoc != -1) glDisableVertexAttribArray(vertexLoc);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
    }
};