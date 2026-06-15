#define GLFW_INCLUDE_NONE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define GLEW_STATIC

// =======================================================================
// --- BIBLIOTEKI ---
// =======================================================================
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h>

// Nasze wlasne pliki i modele
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "map_geometry.h"
#include "city_map.h"
#include "Car.h"
#include "samochodyOBJ.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

// =======================================================================
// --- ZMIENNE GLOBALNE ---
// =======================================================================

// Shadery
ShaderProgram* spMap;
ShaderProgram* sp;

// Stan i ruch gracza
float dist = 0.0f;
float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;
Car autoGracza(0.0f, 0.5f, -2.0f);

// Stan NPC (Inne samochody)
std::vector<Car> inneAuta;
float spawnTimer = 0.0f;
ObjModel modelSamochodu;

// Tekstury
extern GLuint texAsphalt;
extern GLuint texBuilding;
extern GLuint texGrass;
GLuint texSky;

// Stan Gry
bool isCrashed = false;
int cameraMode = 0; // 0 = TPP, 1 = Lot Ptaka, 2 = Pierwsza osoba


// =======================================================================
// --- OBSŁUGA OKNA I KLAWIATURY ---
// =======================================================================

void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Sterowanie ruchem
        if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
        if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
        if (key == GLFW_KEY_UP) speed_y = PI / 2;
        if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;

        // Sterowanie swiatlami auta
        if (key == GLFW_KEY_D) autoGracza.toggleLeftIndicator();
        if (key == GLFW_KEY_A) autoGracza.toggleRightIndicator();
        if (key == GLFW_KEY_S) autoGracza.toggleHazardLights();

        // Restart po zderzeniu
        if (key == GLFW_KEY_SPACE && isCrashed) {
            isCrashed = false;
            glClearColor(0.15f, 0.25f, 0.45f, 1.0f);
            autoGracza.indicatorMode = 0;
            inneAuta.clear();
            spawnTimer = 0.0f;
            PlaySound(TEXT("muzyka.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
        }

        // Zmiana kamery
        if (key == GLFW_KEY_K && action == GLFW_PRESS) {
            cameraMode = (cameraMode + 1) % 3;
        }
    }

    if (action == GLFW_RELEASE) {
        // Zatrzymanie ruchu po puszczeniu klawisza
        if (key == GLFW_KEY_LEFT) speed_x = 0;
        if (key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP) speed_y = 0;
        if (key == GLFW_KEY_DOWN) speed_y = 0;
    }
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}


// =======================================================================
// --- LADOWANIE ZASOBOW (TEKSTURY I SHADERY) ---
// =======================================================================

GLuint readTexture(const char* filename) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::vector<unsigned char> image;
    unsigned width, height;

    unsigned error = lodepng::decode(image, width, height, filename);
    if (error) {
        printf("Blad wczytywania tekstury %s: %s\n", filename, lodepng_error_text(error));
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    return tex;
}

void initOpenGLProgram(GLFWwindow* window) {
    glClearColor(0.15f, 0.25f, 0.45f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
    spMap = new ShaderProgram("v_map.glsl", NULL, "f_map.glsl");

    if (!modelSamochodu.load("Car.obj")) {
        printf("UWAGA: Model nie zostal wczytany!\n");
    }

    // Ustawienie glownego, ambientowego swiatla
    GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glShadeModel(GL_SMOOTH);

    // Wczytanie tekstur srodowiska
    texChodnik = readTexture("chodnik2.png");
    texAsphalt = readTexture("asfalt.png");
    texBuilding = readTexture("kamienica.png");
    texGrass = readTexture("trawa.png");
    texSky = readTexture("niebo.png");

    if (texChodnik == 0) printf("UWAGA: Tekstura chodnika nie zostala wczytana!\n");
    if (texSky == 0) printf("UWAGA: Tekstura nieba nie zostala wczytana!\n");

    // Zabezpieczenie krawedzi tekstury nieba
    glBindTexture(GL_TEXTURE_2D, texSky);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void freeOpenGLProgram(GLFWwindow* window) {
    delete sp;
    delete spMap;
}


// =======================================================================
// --- GLOWNA FUNKCJA RYSOWANIA SCENY ---
// =======================================================================

void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);

    // -------------------------------------------
    // 1. USTAWIANIE KAMERY
    // -------------------------------------------
    glm::mat4 V;
    switch (cameraMode) {
    case 0: // TPP (Z tylu)
        V = glm::lookAt(
            glm::vec3(autoGracza.x * 0.15f, 3.0f, -10.0f),
            glm::vec3(autoGracza.x * 0.3f, 0.0f, 10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        break;
    case 1: // Lot ptaka
        V = glm::lookAt(
            glm::vec3(0.0f, 18.0f, autoGracza.z - 2.0f),
            glm::vec3(0.0f, 0.0f, autoGracza.z + 10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        break;
    case 2: // FPP (Z wewnatrz auta)
        V = glm::lookAt(
            glm::vec3(autoGracza.x, 1.5f, autoGracza.z + 1.15f),
            glm::vec3(autoGracza.x, 1.5f, autoGracza.z + 5.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        break;
    }
    glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.01f, 200.0f);

    // -------------------------------------------
    // 2. RYSOWANIE MAPY MIASTA
    // -------------------------------------------
    spMap->use();
    glUniformMatrix4fv(spMap->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(spMap->u("V"), 1, false, glm::value_ptr(V));
    glUniform1i(spMap->u("isCrashedStatus"), isCrashed ? 1 : 0);

    glm::vec3 globalLampPositions[10];
    int globalLightCount = 0;

    // Funkcja renderujaca z city_map.h
    renderCity(spMap, dist, globalLampPositions, &globalLightCount);
    glUseProgram(0);

    // -------------------------------------------
    // 3. RYSOWANIE AUTA GRACZA (Ze swiatlami miasta)
    // -------------------------------------------
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(P));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(V));

    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Wylaczamy centralne slonce dla pojazdow
    glDisable(GL_LIGHT0);
    GLfloat ambientLight[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    // Zasilanie modelu swiatlami z latarni miejskich
    for (int i = 0; i < globalLightCount && i < 7; i++) {
        glEnable(GL_LIGHT1 + i);
        GLfloat pos[] = { globalLampPositions[i].x, globalLampPositions[i].y, globalLampPositions[i].z, 1.0f };
        GLfloat col[] = { 1.0f, 0.9f, 0.6f, 1.0f };
        glLightfv(GL_LIGHT1 + i, GL_POSITION, pos);
        glLightfv(GL_LIGHT1 + i, GL_DIFFUSE, col);
        glLightf(GL_LIGHT1 + i, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1 + i, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(GL_LIGHT1 + i, GL_QUADRATIC_ATTENUATION, 0.001f);
    }

    // Przesuniecie i narysowanie gracza
    glPushMatrix();
    glTranslatef(autoGracza.x, autoGracza.y, autoGracza.z);
    float katSkretu = speed_x * 15.0f;
    glRotatef(90.0f - katSkretu, 0.0f, 1.0f, 0.0f);
    glScalef(3.0f, 3.0f, 3.0f);
    glTranslatef(-1.0f, -0.25f, -0.4f);

    autoGracza.draw_model_only();
    glPopMatrix();

    // Sprzatanie swiatel
    for (int i = 0; i < globalLightCount && i < 7; i++) {
        glDisable(GL_LIGHT1 + i);
    }
    glEnable(GL_LIGHT0);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);

    // -------------------------------------------
    // 4. RYSOWANIE AUT NPC
    // -------------------------------------------
    for (int i = 0; i < inneAuta.size(); i++) {
        glPushMatrix();
        glTranslatef(inneAuta[i].x, inneAuta[i].y + 0.1f, inneAuta[i].z);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 1.0f, 1.0f);

        sp->use();
        glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
        glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, glm::vec3(inneAuta[i].x, inneAuta[i].y + 0.1f, inneAuta[i].z));
        M = glm::rotate(M, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

        glUniform3fv(sp->u("lightPositions"), globalLightCount, glm::value_ptr(globalLampPositions[0]));
        glUniform1i(sp->u("lightCount"), globalLightCount);

        modelSamochodu.draw(inneAuta[i].colorR, inneAuta[i].colorG, inneAuta[i].colorB);
        glUseProgram(0);

        // Dodawanie prostego, kwadratowego cienia pod NPC
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);

        if (!isCrashed) {
            glBegin(GL_QUADS);
            float npcRoadY = -0.388f;
            glColor4f(1.0f, 1.0f, 0.8f, 0.45f);
            glVertex3f(-0.9f, npcRoadY, 1.2f);
            glVertex3f(0.9f, npcRoadY, 1.2f);
            glColor4f(1.0f, 1.0f, 0.8f, 0.0f);
            glVertex3f(3.0f, npcRoadY, 12.0f);
            glVertex3f(-3.0f, npcRoadY, 12.0f);
            glEnd();
        }

        glDisable(GL_BLEND);
        glPopMatrix();
    }
    glfwSwapBuffers(window);
}


// =======================================================================
// --- SYSTEM KOLIZJI ---
// =======================================================================

bool checkCollision(Car& player, std::vector<Car>& npcs) {
    // Uproszczone ramki kolizji gracza
    float playerMinX = player.x - 0.45f;
    float playerMaxX = player.x + 0.45f;
    float playerMinZ = player.z - 2.0f;
    float playerMaxZ = player.z + 2.9f;

    for (int i = 0; i < npcs.size(); i++) {
        // Ramki kolizji NPC
        float npcMinX = npcs[i].x - 2.0f;
        float npcMaxX = npcs[i].x + 2.0f;
        float npcMinZ = npcs[i].z - 3.0f;
        float npcMaxZ = npcs[i].z + 1.5f;

        bool collisionX = (playerMinX <= npcMaxX) && (playerMaxX >= npcMinX);
        bool collisionZ = (playerMinZ <= npcMaxZ) && (playerMaxZ >= npcMinZ);

        if (collisionX && collisionZ) {
            return true;
        }
    }
    return false;
}


// =======================================================================
// --- PUNKT WEJŚCIA (MAIN LOOP) ---
// =======================================================================

int main(void)
{
    srand(time(NULL));
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "Nie mozna zainicjowac GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);

    if (!window) {
        fprintf(stderr, "Nie mozna utworzyc okna.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Nie mozna zainicjowac GLEW.\n");
        exit(EXIT_FAILURE);
    }

    initOpenGLProgram(window);
    glfwSetTime(0);

    // Odpalenie poczatkowej muzyki
    PlaySound(TEXT("muzyka.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    // --- GLOWNA PETLA GRY ---
    while (!glfwWindowShouldClose(window)) {
        float deltaTime = glfwGetTime();
        glfwSetTime(0);

        if (!isCrashed) {
            // Przesuwanie dystansu (ruch calego swiata do tylu)
            dist += 10.0f * deltaTime;

            // Ruch boczny i przod/tyl pojazdu gracza
            autoGracza.x -= speed_x * deltaTime * 5.0f;
            autoGracza.z += speed_y * deltaTime * 5.0f;

            // Granice pobocza (zabezpieczenie by nie wyjechac za mape)
            if (autoGracza.x < -4.0f) autoGracza.x = -4.0f;
            if (autoGracza.x > 3.5f) autoGracza.x = 3.5f;

            // Granice glebokosci pojazdu na ekranie
            if (autoGracza.z < -6.0f) autoGracza.z = -6.0f;
            if (autoGracza.z > 5.0f) autoGracza.z = 5.0f;

            autoGracza.wheelAngle += 200.0f * deltaTime;

            // GENERATOR AUT NPC
            spawnTimer += deltaTime;
            if (spawnTimer > 3.5f) {
                float lewyPas = -3.0f;
                float prawyPas = 3.0f;
                float laneX = (rand() % 100 < 50) ? lewyPas : prawyPas;

                Car npc(laneX, 0.5f, 40.0f);

                // Losowanie koloru nadwozia
                float paletaBarw[6][3] = {
                    {0.0f, 0.8f, 0.2f},
                    {0.1f, 0.5f, 1.0f},
                    {1.0f, 0.4f, 0.7f},
                    {1.0f, 0.8f, 0.0f},
                    {0.9f, 0.1f, 0.1f},
                    {1.0f, 0.5f, 0.0f}
                };
                int losowyIndeks = rand() % 6;
                npc.colorR = paletaBarw[losowyIndeks][0];
                npc.colorG = paletaBarw[losowyIndeks][1];
                npc.colorB = paletaBarw[losowyIndeks][2];

                inneAuta.push_back(npc);
                spawnTimer = 0.0f;
            }

            // AKTUALIZACJA POZYCJI NPC
            for (int i = 0; i < inneAuta.size(); i++) {
                inneAuta[i].z -= 25.0f * deltaTime; // Szybkosc nadjezdzania z naprzeciwka
                inneAuta[i].wheelAngle -= 200.0f * deltaTime;

                // Usuwamy auta, ktore nas minely, zeby nie obciazac pamieci
                if (inneAuta[i].z < -10.0f) {
                    inneAuta.erase(inneAuta.begin() + i);
                    i--;
                }
            }

            // LOGIKA ZDERZENIA
            if (checkCollision(autoGracza, inneAuta)) {
                isCrashed = true;
                speed_x = 0;
                speed_y = 0;
                glClearColor(0.8f, 0.1f, 0.1f, 1.0f);
                autoGracza.indicatorMode = 3; // Wlacz swiatla awaryjne

                // Odtworz dzwiek stluczki i zatrzymaj tlo muzyczne
                PlaySound(TEXT("crash.wav"), NULL, SND_FILENAME | SND_ASYNC);
            }
        }

        drawScene(window, 0, 0);
        glfwPollEvents();
    }

    freeOpenGLProgram(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}