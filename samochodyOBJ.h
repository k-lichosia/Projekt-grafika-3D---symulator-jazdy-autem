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

// Struktura przechowuj¹ca kolor z pliku .mtl
struct Material {
    float r = 0.8f, g = 0.8f, b = 0.8f; // Domyœlnie szary
};

// Struktura przechowuj¹ca fragment modelu (np. tylko opony, tylko szyby)
struct MeshPart {
    Material mat;
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
                // Kd oznacza 'Diffuse Color' czyli g³ówny kolor materia³u
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
                loadMtl(mtlFilename); // £adujemy plik z kolorami
            }
            else if (type == "usemtl") {
                std::string matName;
                ss >> matName;
                // Tworzymy now¹ czêœæ modelu dla tego koloru
                parts.push_back(MeshPart());
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
                if (parts.empty()) parts.push_back(MeshPart()); // Zabezpieczenie

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

    void draw() {
        // Rysujemy ka¿d¹ czêœæ samochodu osobno, zmieniaj¹c jej kolor
        for (const auto& part : parts) {
            if (part.vertices.empty()) continue;

            // Ustawiamy kolor zdefiniowany w pliku .mtl dla tej konkretnej czêœci
            glColor3f(part.mat.r, part.mat.g, part.mat.b);

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