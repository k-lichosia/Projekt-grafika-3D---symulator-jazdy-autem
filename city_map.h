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

    // Parametry uk³adu (Z³oty œrodek, który ustaliliœmy)
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
                // Pozycja Ÿród³a œwiat³a (¿arówki)
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
        // 1. Aktywujemy jednostkê tekstury i przypisujemy obrazek asfaltu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texAsphalt);
        glUniform1i(sp->u("tex"), 0);

        // 2. W³¹czamy tryb 2 (Asfalt ze struktur¹ i pasami)
        glUniform1i(sp->u("useTexture"), 2);

        // 3. Pozycjonowanie drogi:
        // Poniewa¿ nowa tablica poni¿ej ma ju¿ wbudowany w³aœciwy wymiar,
        // stosujemy translacjê tylko wzd³u¿ osi Z (przesuniêcie klocków mapy)
        M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, shift));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

        // 4. Tworzymy now¹, lokaln¹ definicjê drogi z wbudowanymi UV (X, Y, Z, W, U, V)
        // Droga ma szerokoœæ od -6 do +6 (dok³adnie tyle, ile wynosi Twój roadEdge)
        // oraz d³ugoœæ 100 jednostek (od 0 do 100).
        // Koordynaty U i V powtarzaj¹ siê, daj¹c piêkny efekt ziarnistoœci bez rozci¹gania.
        float texturedRoadVertices[] = {
            // Pierwszy trójk¹t
            -6.0f, 0.0f,   0.0f, 1.0f,   0.0f,  0.0f,
             6.0f, 0.0f,   0.0f, 1.0f,   3.0f,  0.0f,
             6.0f, 0.0f, 100.0f, 1.0f,   3.0f, 25.0f,

             // Drugi trójk¹t
             -6.0f, 0.0f,   0.0f, 1.0f,   0.0f,  0.0f,
              6.0f, 0.0f, 100.0f, 1.0f,   3.0f, 25.0f,
             -6.0f, 0.0f, 100.0f, 1.0f,   0.0f, 25.0f
        };

        // Rysujemy za pomoc¹ drawSimple z flag¹ 'true' na koñcu!
        // Dziêki temu funkcja przeczyta wspó³rzêdne UV z nowej tablicy.
        drawSimple(sp, texturedRoadVertices, NULL, 6, true);

        // --- B. CHODNIKI Z TEKSTUR¥ KOSTKI ---

// 1. Mówimy shaderowi, ¿e teraz u¿ywamy tekstury (uniform z Fragment Shadera)
        glUniform1i(sp->u("useTexture"), 1);

        // 2. Aktywujemy jednostkê teksturuj¹c¹ i przypinamy nasz wczytany plik chodnik.jpg
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texChodnik);
        glUniform1i(sp->u("tex"), 0); // sampler2D w shaderze korzysta z jednostki 0

        // --- PRAWY CHODNIK ---
        M = glm::translate(glm::mat4(1.0f), glm::vec3(roadEdge, 0.01f, shift));
        M = glm::scale(M, glm::vec3(sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

        // Rysujemy: u¿ywamy tablicy sidewalkVertices (tej z dopisanymi u, v)
        // Ostatni parametr 'true' aktywuje w drawSimple czytanie wspó³rzêdnych tekstury
        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // --- LEWY CHODNIK ---
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge, 0.01f, shift));
        // Skalujemy na -sidewalkW, ¿eby "odbiæ" chodnik na lewo
        M = glm::scale(M, glm::vec3(-sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // --- LEWY CHODNIK ---
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge, 0.01f, shift));
        M = glm::scale(M, glm::vec3(-sidewalkW, 1.0f, 1.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

        drawSimple(sp, sidewalkVertices, NULL, 6, true);

        // ====================================================================
        // --- B2. KRAWÊ¯NIKI (Sztuczne rozjaœnienie przez pozycjê Y) ---
        // ====================================================================
        glUniform1i(sp->u("useTexture"), 0);
        glUniform1i(sp->u("lightCount"), lightCount); // Przywracamy normalne œwiat³o

        // PRAWY KRAWÊ¯NIK
        // Trik: podnosimy go minimalnie wy¿ej w skali Y (np. 0.35f), ¿eby góra 
        // ³apa³a wiêcej œwiat³a z góry i by³a jaœniejsza
        M = glm::translate(glm::mat4(1.0f), glm::vec3(roadEdge + 0.1f, 0.17f, shift + 50.0f));
        M = glm::scale(M, glm::vec3(0.2f, 0.35f, 100.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, unitCube, NULL, 36);

        // LEWY KRAWÊ¯NIK
        M = glm::translate(glm::mat4(1.0f), glm::vec3(-roadEdge - 0.1f, 0.17f, shift + 50.0f));
        M = glm::scale(M, glm::vec3(0.2f, 0.35f, 100.0f));
        glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
        drawSimple(sp, unitCube, NULL, 36);

        // --- KONIEC SEKCJI TEKSTUROWANEJ ---
        // Bardzo wa¿ne: wy³¹czamy teksturê, ¿eby budynki i jezdnia jej nie u¿ywa³y!
        glUniform1i(sp->u("useTexture"), 0);

        // --- C. ZAPLECZE / FUNDAMENTY (Bez faktury) ---
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

        // --- D. PASY NA DRODZE ---
        for (int i = 0; i < 100; i += 10) {
            M = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.02f, shift + (float)i));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));
            drawSimple(sp, stripeVertices, whiteStripe, 6);
        }

        glUniform1i(sp->u("useTexture"), 0);

        // --- E. BUDYNKI (Rzadsze i asymetryczne) ---

// LEWA STRONA - rzadziej, co 40 jednostek
        for (int i = 0; i < 100; i += 40) {

            // Startujemy od 10m, ¿eby nie pokrywa³o siê z praw¹ stron¹
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

        // PRAWA STRONA - jeszcze rzadziej, co 55 jednostek
        for (int i = 0; i < 100; i += 55) {
            // Startujemy od 30m - du¿a asymetria wzglêdem lewej strony
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

        // --- F. DRZEWA (Bardziej rozproszone) ---
        // Zwiêkszamy krok z 25 na 45, ¿eby nie by³o ich za gêsto
        for (int i = 0; i < 100; i += 45) {
            // Drzewo Prawe
            M = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, shift + i + 5.0f));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

            glUniform1i(sp->u("useTexture"), 0);

            drawSimple(sp, treeVertices, tCols, treeVertexCount);

            // Drzewo Lewe - przesuniête o 20m wzglêdem prawego
            M = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, shift + i + 25.0f));
            M = glm::scale(M, glm::vec3(0.02f, 0.02f, 0.02f));
            glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, glm::value_ptr(M));

            glUniform1i(sp->u("useTexture"), 0);

            drawSimple(sp, treeVertices, tCols, treeVertexCount);
        }

        // --- F. LATARNIE ---
        for (int i = 0; i < 100; i += 30) {
            float lampX = roadEdge; // Stoj¹ na krawêdzi asfaltu
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
    } // Koniec pêtli bloków
}