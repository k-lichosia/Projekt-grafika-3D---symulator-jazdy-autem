#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "map_geometry.h"
#include "shaderprogram.h"
#include "tree_model.h"

// =======================================================================
// --- ZMIENNE GLOBALNE I TEKSTURY ---
// =======================================================================
GLuint texChodnik;
GLuint texAsphalt;
GLuint texBuilding;
GLuint texGrass;
extern GLuint texSky;

// =======================================================================
// --- FUNKCJE POMOCNICZE (RYSOWANIE I KOLORY) ---
// =======================================================================

// Uniwersalna funkcja do rysowania geometrii (z teksturami lub bez)
void drawSimple(ShaderProgram* sp, float* verts, float* colors, int vertexCount, bool useUV = false) {
    int vPos = sp->a("vertex");
    int cPos = sp->a("color");
    int tPos = sp->a("texCoord");

    if (useUV) {
        // Czytamy z tablicy: [x,y,z,w, u,v] -> krok 6 floatow
        glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), verts);
        glEnableVertexAttribArray(vPos);

        // UV zaczynaja sie po 4 floatach pozycji
        if (tPos != -1) {
            glVertexAttribPointer(tPos, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), verts + 4);
            glEnableVertexAttribArray(tPos);
        }
        glDisableVertexAttribArray(cPos); // Wylaczamy kolor, gdy jest tekstura
    }
    else {
        // Standardowe rysowanie
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

// Generuje kolory dla drzew na podstawie wysokosci wierzcholkow
float* getTreeColors(int count, float* verts) {
    static float* colors = nullptr;
    if (colors == nullptr) {
        colors = new float[count * 4];
        for (int i = 0; i < count; i++) {
            float y = verts[i * 4 + 1];

            if (y < 85.0f) {
                colors[i * 4 + 0] = 0.35f; // R (Brazowy pien)
                colors[i * 4 + 1] = 0.20f; // G
                colors[i * 4 + 2] = 0.10f; // B
            }
            else {
                colors[i * 4 + 0] = 0.05f; // R (Zielona korona)
                colors[i * 4 + 1] = 0.35f; // G
                colors[i * 4 + 2] = 0.05f; // B
            }
            colors[i * 4 + 3] = 1.0f;
        }
    }
    return colors;
}

// Ciemnoszary kolor dla slupow latarni
float* getLampColors() {
    static float colors[36 * 4];
    for (int i = 0; i < 36; i++) {
        colors[i * 4 + 0] = 0.2f; colors[i * 4 + 1] = 0.2f; colors[i * 4 + 2] = 0.2f; colors[i * 4 + 3] = 1.0f;
    }
    return colors;
}

// Jasnozolty kolor dla zrodel swiatla w latarniach
float* getLampLightColors() {
    static float colors[36 * 4];
    for (int i = 0; i < 36; i++) {
        colors[i * 4 + 0] = 1.0f; colors[i * 4 + 1] = 1.0f; colors[i * 4 + 2] = 0.8f; colors[i * 4 + 3] = 1.0f;
    }
    return colors;
}

// Kolor podbudowy krawedznika
float* getCustomCurbColor(float brightness) {
    static float colors[36 * 4];
    for (int i = 0; i < 36; i++) {
        colors[i * 4 + 0] = brightness;
        colors[i * 4 + 1] = brightness;
        colors[i * 4 + 2] = brightness;
        colors[i * 4 + 3] = 1.0f;
    }
    return colors;
}

// =======================================================================
// --- GLOWNA FUNKCJA BUDUJUCA MIASTO ---
// =======================================================================

void renderCity(ShaderProgram* sp, float offset, glm::vec3* outLightPositions = nullptr, int* outLightCount = nullptr) {
    glm::mat4 M;

    // ------------------------------------------
    // 1. OBLICZENIA WSTEPNE
    // ------------------------------------------
    float roadOffset = fmod(offset, 100.0f);
    int treeVertexCount = sizeof(treeVertices) / (4 * sizeof(float));
    float* tCols = getTreeColors(treeVertexCount, treeVertices);

    // Wymiary elementow drogi
    float roadEdge = 6.0f;
    float sidewalkW = 8.0f;
    float foundationW = 30.0f;
    float buildingX = 14.0f;

    float whiteStripe[] = { 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f,
                            0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f, 0.9f, 0.9f, 0.8f, 1.0f };

    // ------------------------------------------
    // 2. PRZYGOTOWANIE SWIATEL LATARNI
    // ------------------------------------------
    glm::vec3 lampPositions[10];
    int lightCount = 0;

    for (int b = 0; b < 2; b++) {
        float s = (b * 100.0f) - roadOffset;
        for (int i = 0; i < 100; i += 30) {
            if (lightCount < 10) {
                lampPositions[lightCount] = glm::vec3(roadEdge - 0.4f, 5.0f, s + (float)i);

                // Zapisujemy pozycje swiatel na zewnatrz (dla aut NPC i Gracza)
                if (outLightPositions != nullptr) {
                    outLightPositions[lightCount] = lampPositions[lightCount];
                }
                lightCount++;
            }
        }
    }

    if (outLightCount != nullptr) {
        *outLightCount = lightCount;
    }

    glUniform3fv(sp->u("lightPositions"), lightCount, glm::value_ptr(lampPositions[0]));
    glUniform1i(sp->u("lightCount"), lightCount);

    // ------------------------------------------
    // 3. NIEBO (SKYBOX)
    // ------------------------------------------
    glDepthMask(GL_FALSE); // Wylaczamy glebie, zeby niebo zawsze bylo "za" miastem

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSky);
    glUniform1i(sp->u("tex"), 0);
    glUniform1i(sp->u("useTexture"), 5); // 5 to nasz tryb dla nieba

    // Tworzymy wielka kostke o boku 190.0f
    glm::mat4 M_sky = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    M_sky = glm::scale(M_sky, glm::vec3(190.0f, 190.0f, 190.0f));
    glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M_sky));

    drawSimple(sp, unitCube, NULL, 36);
    glDepthMask(GL_TRUE); // Wlaczamy glebie z powrotem dla reszty swiata

    // ------------------------------------------
    // 4. GLOWNA PETLA RYSOWANIA MIASTA (Dwa obiekty na raz dla plynnosci)
    // ------------------------------------------
    for (int block = 0; block < 2; block++) {
        float shift = (block * 100.0f) - roadOffset;

        // --- A. JEZDNIA ---
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texAsphalt);
        glUniform1i(sp->u("tex"), 0);
        glUniform1i(sp->u("useTexture"), 2);

        M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, shift));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

        float texturedRoadVertices[] = {
            -6.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
             6.0f, 0.0f, 0.0f, 1.0f, 3.0f, 0.0f,
             6.0f, 0.0f, 100.0f, 1.0f, 3.0f, 25.0f,

            -6.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
             6.0f, 0.0f, 100.0f, 1.0f, 3.0f, 25.0f,
            -6.0f, 0.0f, 100.0f, 1.0f, 0.0f, 25.0f
        };
        drawSimple(sp, texturedRoadVertices, NULL, 6, true);

        // --- B. CHODNIKI ---
        glUniform1i(sp->u("useTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texChodnik);
        glUniform1i(sp->u("tex"), 0);

        // Prawy
        M = glm::translate(glm::mat4(1.0f), glm::vec3(roadEdge, 0.01f, shift));
        M = glm::scale(M, glm::vec3(sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // Lewy
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge, 0.01f, shift));
        M = glm::scale(M, glm::vec3(-sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // --- C. KRAWEDZNIKI ---
        glUniform1i(sp->u("useTexture"), 0);
        glUniform1i(sp->u("lightCount"), lightCount);

        // Prawy
        M = glm::translate(glm::mat4(1.0f), glm::vec3(roadEdge + 0.1f, 0.17f, shift + 50.0f));
        M = glm::scale(M, glm::vec3(0.2f, 0.35f, 100.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, unitCube, NULL, 36);

        // Lewy
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge - 0.1f, 0.17f, shift + 50.0f));
        M = glm::scale(M, glm::vec3(0.2f, 0.35f, 100.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, unitCube, NULL, 36);

        // --- D. ZAPLECZE / FUNDAMENTY MIASTA ---
        M = glm::translate(glm::mat4(1.0f), glm::vec3(buildingX, 0.005f, shift));
        M = glm::scale(M, glm::vec3(foundationW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, foundationVertices, foundationColors, 6);

        M = glm::translate(glm::mat4(1.0f), glm::vec3(-buildingX, 0.005f, shift));
        M = glm::scale(M, glm::vec3(-foundationW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, foundationVertices, foundationColors, 6);

        // --- E. PASY NA DRODZE ---
        for (int i = 0; i < 100; i += 10) {
            M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.02f, shift + (float)i));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, stripeVertices, whiteStripe, 6);
        }

        // --- F. BUDYNKI ---
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texBuilding);
        glUniform1i(sp->u("tex"), 0);
        glUniform1i(sp->u("useTexture"), 3);

        // Lewa strona (co 40 jednostek)
        for (int i = 0; i < 100; i += 40) {
            float leftZ = shift + (float)i + 10.0f;
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-19.0f, 0.0f, leftZ));
            M = glm::scale(M, glm::vec3(10.0f, 20.0f, 10.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, unitCube, buildingColors, 36);
        }

        // Prawa strona (co 55 jednostek)
        for (int i = 0; i < 100; i += 55) {
            float rightZ = shift + (float)i + 30.0f;
            M = glm::translate(glm::mat4(1.0f), glm::vec3(19.0f, 0.0f, rightZ));
            M = glm::scale(M, glm::vec3(10.0f, 20.0f, 10.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, unitCube, buildingColors, 36);
        }

        // --- G. DRZEWA ---
        glUniform1i(sp->u("useTexture"), 0);

        for (int i = 0; i < 100; i += 45) {
            // Prawa strona
            M = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, shift + i + 5.0f));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, treeVertices, tCols, treeVertexCount);

            // Lewa strona
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, shift + i + 25.0f));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, treeVertices, tCols, treeVertexCount);
        }

        // --- H. LATARNIE ---
        for (int i = 0; i < 100; i += 30) {
            float lampX = roadEdge;
            float lampZ = shift + (float)i;

            // Slup latarni
            M = glm::translate(glm::mat4(1.0f), glm::vec3(lampX, 2.5f, lampZ));
            M = glm::scale(M, glm::vec3(0.15f, 5.0f, 0.15f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, unitCube, getLampColors(), 36);

            // Glowica emitujaca swiatlo
            M = glm::translate(glm::mat4(1.0f), glm::vec3(lampX - 0.4f, 5.0f, lampZ));
            M = glm::scale(M, glm::vec3(0.8f, 0.15f, 0.2f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, unitCube, getLampLightColors(), 36);
        }

        // --- I. TRAWNIKI MIEDZY BUDYNKAMI ---
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texGrass);
        glUniform1i(sp->u("tex"), 0);
        glUniform1i(sp->u("useTexture"), 4);

        float grassY = 0.02f;

        // Lewa strona (Zsynchronizowane z budynkami)
        for (int i = 0; i < 100; i += 40) {
            float leftZ = shift + (float)i + 10.0f;
            glm::mat4 mLeft1 = glm::translate(glm::mat4(1.0f), glm::vec3(-23.0f, grassY, leftZ));
            mLeft1 = glm::scale(mLeft1, glm::vec3(18.0f, 0.01f, 38.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(mLeft1));
            drawSimple(sp, unitCube, NULL, 36);
        }

        // Prawa strona
        for (int i = 0; i < 100; i += 55) {
            float rightZ = shift + (float)i + 30.0f;
            glm::mat4 mRight1 = glm::translate(glm::mat4(1.0f), glm::vec3(23.0f, grassY, rightZ));
            mRight1 = glm::scale(mRight1, glm::vec3(18.0f, 0.01f, 53.0f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(mRight1));
            drawSimple(sp, unitCube, NULL, 36);
        }

        glUniform1i(sp->u("useTexture"), 0);

    }
}