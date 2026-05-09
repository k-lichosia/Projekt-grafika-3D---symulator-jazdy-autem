#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "map_geometry.h"
#include "shaderprogram.h"
#include "tree_model.h"

// Uniwersalna funkcja do rysowania
void drawSimple(ShaderProgram* sp, float* verts, float* colors, int vertexCount) {
    int vPos = sp->a("vertex");
    int cPos = sp->a("color");

    glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(vPos);

    glVertexAttribPointer(cPos, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(cPos);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glDisableVertexAttribArray(vPos);
    glDisableVertexAttribArray(cPos);
}

float* getTreeColors(int count, float* verts) {
    static float* colors = nullptr;
    if (colors == nullptr) {
        colors = new float[count * 4];
        for (int i = 0; i < count; i++) {
            float y = verts[i * 4 + 1]; // To jest wysokoœæ wierzcho³ka

            // POPRAWKA: Zwiêkszamy próg do 85.0f
            if (y < 85.0f) {
                colors[i * 4 + 0] = 0.35f; // R (Br¹zowy)
                colors[i * 4 + 1] = 0.20f; // G
                colors[i * 4 + 2] = 0.10f; // B
            }
            else {
                colors[i * 4 + 0] = 0.05f; // R (Ciemniejszy zielony)
                colors[i * 4 + 1] = 0.35f; // G
                colors[i * 4 + 2] = 0.05f; // B
            }
            colors[i * 4 + 3] = 1.0f;
        }
    }
    return colors;
}

void renderCity(ShaderProgram* sp, float offset) {
    glm::mat4 M;

    // 1. OBLICZAMY OFFSET CYKLICZNY
    float roadOffset = fmod(offset, 100.0f);

    // 1. Obliczamy liczbê wierzcho³ków (jeœli jeszcze tego nie zrobi³aœ)
    int treeVertexCount = sizeof(treeVertices) / (4 * sizeof(float));

    float* tCols = getTreeColors(treeVertexCount, treeVertices);

    float whiteStripe[] = { 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f,
                            0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f };

    for (int block = 0; block < 2; block++) {
        float shift = (block * 100.0f) - roadOffset;

        // --- A. DROGA ---
        M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, shift));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, roadVertices, roadColors, 6);

        // --- B. CHODNIKI ---
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 0.0f, shift));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, curbVertices, curbColors, 6);

        M = glm::translate(glm::mat4(1.0f), glm::vec3(8.0f, 0.0f, shift));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, curbVertices, curbColors, 6);

        // --- C. PASY ---
        for (int i = 0; i < 100; i += 10) {
            M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.01f, shift + (float)i));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, stripeVertices, whiteStripe, 6);
        }

        // --- D. BUDYNKI I E. DRZEWA ---
        for (int i = 0; i < 100; i += 40) {
            // Budynek lewy
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, 0.0f, shift + (float)i));
            glm::mat4 M_leftB = glm::scale(M, glm::vec3(2.0f, 1.5f, 3.5f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M_leftB));
            drawSimple(sp, buildingVertices, buildingColors, 30);


            // DRZEWO LEWE (w luce 20m za budynkiem)
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 0.0f, shift + (float)i + 20.0f));
            // TU DOPASUJ SKALÊ: Jeœli drzewo jest wielkie, zmieñ 1.0f na 0.1f. Jeœli ma³e, na 5.0f.
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, treeVertices, tCols, treeVertexCount);

            // Budynek prawy (przesuniêty o 20m)
            M = glm::translate(glm::mat4(1.0f), glm::vec3(14.0f, 0.0f, shift + (float)i + 20.0f));
            glm::mat4 M_rightB = glm::scale(M, glm::vec3(2.0f, 2.0f, 3.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M_rightB));
            drawSimple(sp, buildingVertices, buildingColors, 30);

            // DRZEWO PRAWE (w luce przed budynkiem)
            M = glm::translate(glm::mat4(1.0f), glm::vec3(8.0f, 0.0f, shift + (float)i));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, treeVertices, tCols, treeVertexCount);
        }
    }
}
