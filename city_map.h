#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "map_geometry.h"
#include "shaderprogram.h"
#include "tree_model.h"

GLuint texChodnik;
GLuint texAsphalt;
GLuint texBuilding;
GLuint texGrass;

// Uniwersalna funkcja do rysowania
void drawSimple(ShaderProgram* sp, float* verts, float* colors, int vertexCount, bool useUV = false) {
    int vPos = sp->a("vertex");
    int cPos = sp->a("color");
    int tPos = sp->a("texCoord"); // Pobierz lokalizacjê UV z shadera

    if (useUV) {
        // Czytamy z tablicy: [x,y,z,w, u,v] -> krok 6 floatów
        glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), verts);
        glEnableVertexAttribArray(vPos);

        // UV zaczynaj¹ siê po 4 floatach pozycji
        if (tPos != -1) {
            glVertexAttribPointer(tPos, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), verts + 4);
            glEnableVertexAttribArray(tPos);
        }
        glDisableVertexAttribArray(cPos); // Wy³¹cz kolor, gdy jest tekstura
    }
    else {
        // Standardowe rysowanie (krok 0, bo verts i colors s¹ osobno)
        glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 0, verts);
        glEnableVertexAttribArray(vPos);

        if (colors != nullptr) {
            glVertexAttribPointer(cPos, 4, GL_FLOAT, GL_FALSE, 0, colors);
            glEnableVertexAttribArray(cPos);
        }
        if (tPos != -1) glDisableVertexAttribArray(tPos);
    }

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glDisableVertexAttribArray(vPos);
    if (cPos != -1) glDisableVertexAttribArray(cPos);
    if (tPos != -1) glDisableVertexAttribArray(tPos);
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

float* getLampColors() {
    static float colors[36 * 4];
    for (int i = 0; i < 36; i++) {
        colors[i * 4 + 0] = 0.2f; colors[i * 4 + 1] = 0.2f; colors[i * 4 + 2] = 0.2f; colors[i * 4 + 3] = 1.0f;
    }
    return colors;
}

float* getLampLightColors() {
    static float colors[36 * 4];
    for (int i = 0; i < 36; i++) {
        colors[i * 4 + 0] = 1.0f; colors[i * 4 + 1] = 1.0f; colors[i * 4 + 2] = 0.8f; colors[i * 4 + 3] = 1.0f;
    }
    return colors;
}

float* getCustomCurbColor(float brightness) {
    static float colors[36 * 4];
    for (int i = 0; i < 36; i++) {
        colors[i * 4 + 0] = brightness; // R
        colors[i * 4 + 1] = brightness; // G
        colors[i * 4 + 2] = brightness; // B
        colors[i * 4 + 3] = 1.0f;       // A
    }
    return colors;
}

void renderCity(ShaderProgram* sp, float offset) {
    glm::mat4 M;

    // 1. OBLICZENIA WSTÊPNE
    float roadOffset = fmod(offset, 100.0f);
    int treeVertexCount = sizeof(treeVertices) / (4 * sizeof(float));
    float* tCols = getTreeColors(treeVertexCount, treeVertices);

    // Parametry uk³adu (Z³oty œrodek)
    float roadEdge = 6.0f;      // Koniec asfaltu
    float sidewalkW = 8.0f;     // Szerokoœæ chodnika (kostki)
    float foundationW = 30.0f;  // Szerokoœæ zaplecza
    float buildingX = 14.0f;    // Linia zabudowy (6 + 8)

    float whiteStripe[] = { 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f,
                            0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f };

    // 2. PRZYGOTOWANIE ŒWIATE£ (Uniformy)
    glm::vec3 lampPositions[10];
    int lightCount = 0;
    for (int b = 0; b < 2; b++) {
        float s = (b * 100.0f) - roadOffset;
        for (int i = 0; i < 100; i += 30) {
            if (lightCount < 10) {
                lampPositions[lightCount] = glm::vec3(roadEdge - 0.4f, 5.0f, s + (float)i);
                lightCount++;
            }
        }
    }
    glUniform3fv(sp->u("lightPositions"), lightCount, glm::value_ptr(lampPositions[0]));
    glUniform1i(sp->u("lightCount"), lightCount);

    // 3. G£ÓWNA PÊTLA RYSOWANIA (Dwa bloki miasta dla p³ynnoœci ruchu)
    for (int block = 0; block < 2; block++) {
        float shift = (block * 100.0f) - roadOffset;

        // ====================================================================
        // --- B1. JEZDNIA (ZAAWANSOWANE NAK£ADANIE STRUKTURY) ---
        // ====================================================================
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texAsphalt);
        glUniform1i(sp->u("tex"), 0);
        glUniform1i(sp->u("useTexture"), 2);

        M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, shift));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

        float texturedRoadVertices[] = {
            -6.0f, 0.0f,   0.0f, 1.0f,   0.0f,  0.0f,
             6.0f, 0.0f,   0.0f, 1.0f,   3.0f,  0.0f,
             6.0f, 0.0f, 100.0f, 1.0f,   3.0f, 25.0f,

             -6.0f, 0.0f,   0.0f, 1.0f,   0.0f,  0.0f,
              6.0f, 0.0f, 100.0f, 1.0f,   3.0f, 25.0f,
             -6.0f, 0.0f, 100.0f, 1.0f,   0.0f, 25.0f
        };
        drawSimple(sp, texturedRoadVertices, NULL, 6, true);

        // ====================================================================
        // --- B2. CHODNIKI Z TEKSTUR¥ KOSTKI ---
        // ====================================================================
        glUniform1i(sp->u("useTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texChodnik);
        glUniform1i(sp->u("tex"), 0);

        // Prawy chodnik
        M = glm::translate(glm::mat4(1.0f), glm::vec3(roadEdge, 0.01f, shift));
        M = glm::scale(M, glm::vec3(sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // Lewy chodnik
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge, 0.01f, shift));
        M = glm::scale(M, glm::vec3(-sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // ====================================================================
        // --- B3. KRAWÊ¯NIKI ---
        // ====================================================================
        glUniform1i(sp->u("useTexture"), 0);
        glUniform1i(sp->u("lightCount"), lightCount);

        // Prawy krawê¿nik
        M = glm::translate(glm::mat4(1.0f), glm::vec3(roadEdge + 0.1f, 0.17f, shift + 50.0f));
        M = glm::scale(M, glm::vec3(0.2f, 0.35f, 100.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, unitCube, NULL, 36);

        // Lewy krawê¿nik
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge - 0.1f, 0.17f, shift + 50.0f));
        M = glm::scale(M, glm::vec3(0.2f, 0.35f, 100.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, unitCube, NULL, 36);

        glUniform1i(sp->u("useTexture"), 0);

        // ====================================================================
        // --- C. ZAPLECZE / FUNDAMENTY (Bez faktury) ---
        // ====================================================================
        // Prawy
        M = glm::translate(glm::mat4(1.0f), glm::vec3(buildingX, 0.005f, shift));
        M = glm::scale(M, glm::vec3(foundationW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, foundationVertices, foundationColors, 6);

        // Lewy
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-buildingX, 0.005f, shift));
        M = glm::scale(M, glm::vec3(-foundationW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, foundationVertices, foundationColors, 6);

        // ====================================================================
        // --- D. PASY NA DRODZE ---
        // ====================================================================
        for (int i = 0; i < 100; i += 10) {
            M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.02f, shift + (float)i));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, stripeVertices, whiteStripe, 6);
        }

        glUniform1i(sp->u("useTexture"), 0);

        // ====================================================================
        // --- E. BUDYNKI (Rzadsze i asymetryczne) ---
        // ====================================================================
        // Lewa strona - co 40 jednostek
        for (int i = 0; i < 100; i += 40) {
            float leftZ = shift + (float)i + 10.0f;
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-19.0f, 0.0f, leftZ));
            M = glm::scale(M, glm::vec3(10.0f, 20.0f, 10.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texBuilding);
            glUniform1i(sp->u("tex"), 0);
            glUniform1i(sp->u("useTexture"), 3);
            drawSimple(sp, unitCube, buildingColors, 36);
        }

        // Prawa strona - co 55 jednostek
        for (int i = 0; i < 100; i += 55) {
            float rightZ = shift + (float)i + 30.0f;
            M = glm::translate(glm::mat4(1.0f), glm::vec3(19.0f, 0.0f, rightZ));
            M = glm::scale(M, glm::vec3(10.0f, 20.0f, 10.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texBuilding);
            glUniform1i(sp->u("tex"), 0);
            glUniform1i(sp->u("useTexture"), 3);
            drawSimple(sp, unitCube, buildingColors, 36);
        }

        // ====================================================================
        // --- F. DRZEWA (Bardziej rozproszone) ---
        // ====================================================================
        for (int i = 0; i < 100; i += 45) {
            // Drzewo Prawe
            M = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, shift + i + 5.0f));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            glUniform1i(sp->u("useTexture"), 0);
            drawSimple(sp, treeVertices, tCols, treeVertexCount);

            // Drzewo Lewe
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, shift + i + 25.0f));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            glUniform1i(sp->u("useTexture"), 0);
            drawSimple(sp, treeVertices, tCols, treeVertexCount);
        }

        // ====================================================================
        // --- G. LATARNIE ---
        // ====================================================================
        for (int i = 0; i < 100; i += 30) {
            float lampX = roadEdge;
            float lampZ = shift + (float)i;

            // S³up
            M = glm::translate(glm::mat4(1.0f), glm::vec3(lampX, 2.5f, lampZ));
            M = glm::scale(M, glm::vec3(0.15f, 5.0f, 0.15f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            glUniform1i(sp->u("useTexture"), 0);
            drawSimple(sp, unitCube, getLampColors(), 36);

            // G³owica
            M = glm::translate(glm::mat4(1.0f), glm::vec3(lampX - 0.4f, 5.0f, lampZ));
            M = glm::scale(M, glm::vec3(0.8f, 0.15f, 0.2f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            glUniform1i(sp->u("useTexture"), 0);
            drawSimple(sp, unitCube, getLampLightColors(), 36);
        }

        // ====================================================================
    // --- ROZSZERZONE TRAWNIKI WZD£U¯ OSI JEZDNI (MIÊDZY BUDYNKAMI) ---
    // ====================================================================

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texGrass);
        glUniform1i(sp->u("tex"), 0);
        glUniform1i(sp->u("useTexture"), 4);

        float grassY = 0.02f;

        // --- 1. LEWA STRONA ULICY (Pêtla co 40 jednostek) ---
        for (int i = 0; i < 100; i += 40) {
            float leftZ = shift + (float)i + 10.0f;

            // Szerokoœæ wraca do 18.0f (czysty styk z chodnikiem), 
            // ale D£UGOŒÆ zwiêkszamy do 38.0f (zamiast 14.0f).
            // Poniewa¿ pêtla przeskakuje co 40, trawniki zepn¹ siê w niemal ci¹g³y pas wzd³u¿ drogi!
            glm::mat4 mLeft1 = glm::translate(glm::mat4(1.0f), glm::vec3(-23.0f, grassY, leftZ));
            mLeft1 = glm::scale(mLeft1, glm::vec3(18.0f, 0.01f, 38.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(mLeft1));
            drawSimple(sp, unitCube, NULL, 36);
        }

        // --- 2. PRAWA STRONA ULICY (Pêtla co 55 jednostek) ---
        for (int i = 0; i < 100; i += 55) {
            float rightZ = shift + (float)i + 30.0f;

            // Szerokoœæ 18.0f, a D£UGOŒÆ zwiêkszamy do 53.0f (zamiast 14.0f).
            // Budynki po prawej rzadziej siê powtarzaj¹ (co 55), wiêc trawa o d³ugoœci 53 
            // idealnie wype³ni gigantyczne czarne luki miêdzy nimi wzd³u¿ chodnika.
            glm::mat4 mRight1 = glm::translate(glm::mat4(1.0f), glm::vec3(23.0f, grassY, rightZ));
            mRight1 = glm::scale(mRight1, glm::vec3(18.0f, 0.01f, 53.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(mRight1));
            drawSimple(sp, unitCube, NULL, 36);
        }

        glUniform1i(sp->u("useTexture"), 0);

    } // Koniec pêtli 'block'
} // Koniec funkcji 'renderCity'