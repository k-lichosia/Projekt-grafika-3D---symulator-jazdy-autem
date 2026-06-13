#ifndef CAR_H
#define CAR_H

#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderprogram.h"

class Car {
public:
    // Wewnętrzne narzędzie do nowoczesnego OpenGL - automatycznie tnie na trójkąty i liczy światło
    struct CarPart {
        GLuint vao = 0, vboVerts = 0, vboNorms = 0;
        std::vector<float> vertices;
        std::vector<float> normals;

        void addTri(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
            glm::vec3 u = p2 - p1;
            glm::vec3 v = p3 - p1;
            glm::vec3 n = glm::cross(u, v);
            if (glm::length(n) > 0.0001f) n = glm::normalize(n);
            else n = glm::vec3(0, 1, 0); // Zabezpieczenie przed błędem zera

            vertices.insert(vertices.end(), { p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z });
            normals.insert(normals.end(), { n.x, n.y, n.z, n.x, n.y, n.z, n.x, n.y, n.z });
        }

        void addQuad(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) {
            addTri(p1, p2, p3);
            addTri(p1, p3, p4);
        }

        void build(ShaderProgram* sp) {
            if (vertices.empty()) return;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glGenBuffers(1, &vboVerts);
            glBindBuffer(GL_ARRAY_BUFFER, vboVerts);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            GLint vLoc = sp->a("vertex");
            if (vLoc != -1) { glEnableVertexAttribArray(vLoc); glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL); }

            glGenBuffers(1, &vboNorms);
            glBindBuffer(GL_ARRAY_BUFFER, vboNorms);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
            GLint nLoc = sp->a("normal");
            if (nLoc != -1) { glEnableVertexAttribArray(nLoc); glVertexAttribPointer(nLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL); }

            glBindVertexArray(0);
        }

        void draw() {
            if (vao == 0) return;
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
            glBindVertexArray(0);
        }
    };

    float x, y, z;
    float wheelAngle;
    float r, g, b;
    int indicatorMode = 0;

    // Nasze dwie nowoczesne części oświetlane z Shadera
    CarPart bodyMesh;
    CarPart glassMesh;

    Car(float startX = 0.0f, float startY = 0.0f, float startZ = 0.0f)
        : x(startX), y(startY), z(startZ), wheelAngle(0.0f), r(0.2f), g(0.4f), b(1.0f) {
    }

    // Dodaj to w sekcji public:
    void setColor(float red, float green, float blue) {
        r = red;
        g = green;
        b = blue;
    }

    void toggleLeftIndicator() { indicatorMode = (indicatorMode == 1) ? 0 : 1; }
    void toggleRightIndicator() { indicatorMode = (indicatorMode == 2) ? 0 : 2; }
    void toggleHazardLights() { indicatorMode = (indicatorMode == 3) ? 0 : 3; }

    void move(float speed) {
        x += speed;
        wheelAngle += speed * 50.0f;
    }

    // ==========================================
    // 1. ŁADOWANIE KAROSERII DO KARTY GRAFICZNEJ (ZADZIAŁA RAZ)
    // ==========================================
    void initModern(ShaderProgram* sp) {
        // --- Karoseria --- (Twoje współrzędne przetłumaczone na nowy system!)
        bodyMesh.addQuad(glm::vec3(0.2f, 0.4f, 0.6f), glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(0.6f, 0.5f, 0.2f), glm::vec3(0.2f, 0.4f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.2f, 0.4f, 0.6f), glm::vec3(0.6f, 0.2f, 0.6f), glm::vec3(0.6f, 0.2f, 0.2f), glm::vec3(0.2f, 0.2f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.2f, 0.2f, 0.6f), glm::vec3(0.2f, 0.4f, 0.6f), glm::vec3(0.2f, 0.4f, 0.2f), glm::vec3(0.2f, 0.2f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.6f, 0.2f, 0.6f), glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(0.6f, 0.5f, 0.2f), glm::vec3(0.6f, 0.2f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.2f, 0.2f, 0.6f), glm::vec3(0.6f, 0.2f, 0.6f), glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(0.2f, 0.4f, 0.6f));
        bodyMesh.addQuad(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.6f, 0.2f, 0.2f), glm::vec3(0.6f, 0.5f, 0.2f), glm::vec3(0.2f, 0.4f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.7f, 0.65f, 0.6f), glm::vec3(0.7f, 0.65f, 0.2f), glm::vec3(1.7f, 0.65f, 0.2f), glm::vec3(1.7f, 0.65f, 0.6f));

        bodyMesh.addQuad(glm::vec3(1.8f, 0.5f, 0.6f), glm::vec3(1.8f, 0.5f, 0.2f), glm::vec3(2.1f, 0.4f, 0.2f), glm::vec3(2.1f, 0.4f, 0.6f));
        bodyMesh.addQuad(glm::vec3(2.1f, 0.2f, 0.6f), glm::vec3(2.1f, 0.2f, 0.2f), glm::vec3(1.8f, 0.2f, 0.6f), glm::vec3(1.8f, 0.2f, 0.6f));
        bodyMesh.addQuad(glm::vec3(2.1f, 0.4f, 0.6f), glm::vec3(2.1f, 0.4f, 0.2f), glm::vec3(2.1f, 0.2f, 0.2f), glm::vec3(2.1f, 0.2f, 0.6f));
        bodyMesh.addQuad(glm::vec3(1.8f, 0.2f, 0.2f), glm::vec3(1.8f, 0.5f, 0.2f), glm::vec3(2.1f, 0.4f, 0.2f), glm::vec3(2.1f, 0.2f, 0.2f));
        bodyMesh.addQuad(glm::vec3(1.8f, 0.2f, 0.6f), glm::vec3(1.8f, 0.5f, 0.6f), glm::vec3(2.1f, 0.4f, 0.6f), glm::vec3(2.1f, 0.2f, 0.6f));

        bodyMesh.addQuad(glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(0.6f, 0.2f, 0.6f), glm::vec3(1.8f, 0.2f, 0.6f), glm::vec3(1.8f, 0.5f, 0.6f));
        bodyMesh.addQuad(glm::vec3(0.6f, 0.2f, 0.6f), glm::vec3(0.6f, 0.2f, 0.2f), glm::vec3(1.8f, 0.2f, 0.2f), glm::vec3(1.8f, 0.2f, 0.6f));
        bodyMesh.addQuad(glm::vec3(0.6f, 0.5f, 0.2f), glm::vec3(0.6f, 0.2f, 0.2f), glm::vec3(1.8f, 0.2f, 0.2f), glm::vec3(1.8f, 0.5f, 0.2f));

        bodyMesh.addQuad(glm::vec3(0.7f, 0.65f, 0.2f), glm::vec3(0.7f, 0.5f, 0.2f), glm::vec3(0.75f, 0.5f, 0.2f), glm::vec3(0.77f, 0.65f, 0.2f));
        bodyMesh.addQuad(glm::vec3(1.2f, 0.65f, 0.2f), glm::vec3(1.2f, 0.5f, 0.2f), glm::vec3(1.25f, 0.5f, 0.2f), glm::vec3(1.27f, 0.65f, 0.2f));
        bodyMesh.addQuad(glm::vec3(1.65f, 0.65f, 0.2f), glm::vec3(1.65f, 0.5f, 0.2f), glm::vec3(1.7f, 0.5f, 0.2f), glm::vec3(1.7f, 0.65f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.75f, 0.65f, 0.2f), glm::vec3(0.75f, 0.63f, 0.2f), glm::vec3(1.7f, 0.63f, 0.2f), glm::vec3(1.7f, 0.65f, 0.2f));
        bodyMesh.addQuad(glm::vec3(0.75f, 0.65f, 0.6f), glm::vec3(0.75f, 0.63f, 0.6f), glm::vec3(1.7f, 0.63f, 0.6f), glm::vec3(1.7f, 0.65f, 0.6f));
        bodyMesh.build(sp);

        // --- Szyby ---
        glassMesh.addQuad(glm::vec3(0.77f, 0.63f, 0.2f), glm::vec3(0.75f, 0.5f, 0.2f), glm::vec3(1.2f, 0.5f, 0.2f), glm::vec3(1.22f, 0.63f, 0.2f));
        glassMesh.addQuad(glm::vec3(1.27f, 0.63f, 0.2f), glm::vec3(1.25f, 0.5f, 0.2f), glm::vec3(1.65f, 0.5f, 0.2f), glm::vec3(1.67f, 0.63f, 0.2f));
        glassMesh.addQuad(glm::vec3(0.77f, 0.63f, 0.6f), glm::vec3(0.75f, 0.5f, 0.6f), glm::vec3(1.2f, 0.5f, 0.6f), glm::vec3(1.22f, 0.63f, 0.6f));
        glassMesh.addQuad(glm::vec3(1.27f, 0.63f, 0.6f), glm::vec3(1.25f, 0.5f, 0.6f), glm::vec3(1.65f, 0.5f, 0.6f), glm::vec3(1.67f, 0.63f, 0.6f));
        glassMesh.addQuad(glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(0.6f, 0.5f, 0.2f), glm::vec3(0.7f, 0.65f, 0.2f), glm::vec3(0.7f, 0.65f, 0.6f));
        glassMesh.addQuad(glm::vec3(1.7f, 0.65f, 0.6f), glm::vec3(1.7f, 0.65f, 0.2f), glm::vec3(1.8f, 0.5f, 0.2f), glm::vec3(1.8f, 0.5f, 0.6f));
        glassMesh.addTri(glm::vec3(0.6f, 0.5f, 0.6f), glm::vec3(0.7f, 0.65f, 0.6f), glm::vec3(0.7f, 0.5f, 0.6f));
        glassMesh.addTri(glm::vec3(0.6f, 0.5f, 0.2f), glm::vec3(0.7f, 0.65f, 0.2f), glm::vec3(0.7f, 0.5f, 0.2f));
        glassMesh.addTri(glm::vec3(1.7f, 0.65f, 0.2f), glm::vec3(1.8f, 0.5f, 0.2f), glm::vec3(1.7f, 0.5f, 0.2f));
        glassMesh.addTri(glm::vec3(1.7f, 0.65f, 0.6f), glm::vec3(1.8f, 0.5f, 0.6f), glm::vec3(1.7f, 0.5f, 0.6f));
        glassMesh.build(sp);
    }

    // ==========================================
    // 2. NOWOCZESNE RYSOWANIE SHADERAMI (Karoseria)
    // ==========================================
    void drawModern(ShaderProgram* sp, glm::mat4 M) {
        glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

        glUniform3f(sp->u("objectColor"), r, g, b);
        bodyMesh.draw();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform3f(sp->u("objectColor"), 0.5f, 0.8f, 1.0f);
        glassMesh.draw();
        glDisable(GL_BLEND);
    }

    // ==========================================
    // 3. STARE ZASADY - TYLKO DLA ŚWIATEŁ I KÓŁ
    // ==========================================
    void draw_accessories() {
        bool flashOn = ((int)(clock() / 400) % 2) == 0;
        bool showLeft = flashOn && (indicatorMode == 1 || indicatorMode == 3);
        bool showRight = flashOn && (indicatorMode == 2 || indicatorMode == 3);

        GLfloat noGlow[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        // ŚWIATŁA
        GLfloat glowLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, glowLight);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(0.199f, 0.35f, 0.25f); glVertex3f(0.199f, 0.35f, 0.33f); glVertex3f(0.199f, 0.27f, 0.33f); glVertex3f(0.199f, 0.27f, 0.25f);
        glVertex3f(0.199f, 0.35f, 0.47f); glVertex3f(0.199f, 0.35f, 0.55f); glVertex3f(0.199f, 0.27f, 0.55f); glVertex3f(0.199f, 0.27f, 0.47f);
        glEnd();

        GLfloat rearGlowLight[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, rearGlowLight);
        glColor3f(1.0f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
        glVertex3f(2.101f, 0.35f, 0.25f); glVertex3f(2.101f, 0.35f, 0.33f); glVertex3f(2.101f, 0.27f, 0.33f); glVertex3f(2.101f, 0.27f, 0.25f);
        glVertex3f(2.101f, 0.35f, 0.47f); glVertex3f(2.101f, 0.35f, 0.55f); glVertex3f(2.101f, 0.27f, 0.55f); glVertex3f(2.101f, 0.27f, 0.47f);
        glEnd();

        GLfloat orangeGlow[] = { 1.0f, 0.5f, 0.0f, 1.0f };
        if (showLeft) {
            glMaterialfv(GL_FRONT, GL_EMISSION, orangeGlow); glColor3f(1.0f, 0.5f, 0.0f);
            glBegin(GL_QUADS);
            glVertex3f(0.198f, 0.34f, 0.20f); glVertex3f(0.198f, 0.34f, 0.24f); glVertex3f(0.198f, 0.28f, 0.24f); glVertex3f(0.198f, 0.28f, 0.20f);
            glVertex3f(2.102f, 0.34f, 0.20f); glVertex3f(2.102f, 0.34f, 0.24f); glVertex3f(2.102f, 0.28f, 0.24f); glVertex3f(2.102f, 0.28f, 0.20f);
            glEnd();
        }
        if (showRight) {
            glMaterialfv(GL_FRONT, GL_EMISSION, orangeGlow); glColor3f(1.0f, 0.5f, 0.0f);
            glBegin(GL_QUADS);
            glVertex3f(0.198f, 0.34f, 0.56f); glVertex3f(0.198f, 0.34f, 0.59f); glVertex3f(0.198f, 0.28f, 0.59f); glVertex3f(0.198f, 0.28f, 0.56f);
            glVertex3f(2.102f, 0.34f, 0.56f); glVertex3f(2.102f, 0.34f, 0.59f); glVertex3f(2.102f, 0.28f, 0.59f); glVertex3f(2.102f, 0.28f, 0.56f);
            glEnd();
        }
        glMaterialfv(GL_FRONT, GL_EMISSION, noGlow);

        // POŚWIATY (Asfalt)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        float roadY = 0.015f;
        glBegin(GL_QUADS);
        float glowStart = 2.1f, glowEnd = 2.8f;
        glColor4f(1.0f, 0.0f, 0.0f, 0.25f); glVertex3f(glowStart, roadY, 0.23f); glVertex3f(glowStart, roadY, 0.35f);
        glColor4f(1.0f, 0.0f, 0.0f, 0.0f);  glVertex3f(glowEnd, roadY, 0.35f);   glVertex3f(glowEnd, roadY, 0.23f);
        glColor4f(1.0f, 0.0f, 0.0f, 0.25f); glVertex3f(glowStart, roadY, 0.45f); glVertex3f(glowStart, roadY, 0.57f);
        glColor4f(1.0f, 0.0f, 0.0f, 0.0f);  glVertex3f(glowEnd, roadY, 0.57f);   glVertex3f(glowEnd, roadY, 0.45f);

        if (showLeft) {
            glColor4f(1.0f, 0.5f, 0.0f, 0.25f); glVertex3f(-0.05f, roadY, 0.18f); glVertex3f(-0.05f, roadY, 0.26f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.0f);  glVertex3f(-0.75f, roadY, 0.26f); glVertex3f(-0.75f, roadY, 0.18f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.25f); glVertex3f(2.1f, roadY, 0.18f);  glVertex3f(2.1f, roadY, 0.26f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.0f);  glVertex3f(2.7f, roadY, 0.26f);  glVertex3f(2.7f, roadY, 0.18f);
        }
        if (showRight) {
            glColor4f(1.0f, 0.5f, 0.0f, 0.25f); glVertex3f(-0.05f, roadY, 0.53f);  glVertex3f(-0.05, roadY, 0.61f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.0f);  glVertex3f(-0.75f, roadY, 0.61f); glVertex3f(-0.75f, roadY, 0.53f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.25f); glVertex3f(2.1f, roadY, 0.53f);  glVertex3f(2.1f, roadY, 0.61f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.0f);  glVertex3f(2.7f, roadY, 0.61f);  glVertex3f(2.7f, roadY, 0.53f);
        }
        glEnd();
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);

        // WYDECH
        glPushMatrix();
        glColor3f(0.4f, 0.4f, 0.4f);
        glTranslatef(1.65f, 0.2f, 0.3f);
        glRotatef(90.0f, 0, 1, 0);
        drawExhaust(0.02f, 0.03f, 0.5f);
        glPopMatrix();

        // KOŁA
        glColor3f(0.7f, 0.7f, 0.7f);
        float theta;
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) { glVertex3f(0.6f, 0.2f, 0.62f); glVertex3f(0.6f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.62f); }
        glEnd();
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) { glVertex3f(0.6f, 0.2f, 0.18f); glVertex3f(0.6f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.18f); }
        glEnd();
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) { glVertex3f(1.7f, 0.2f, 0.18f); glVertex3f(1.7f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.18f); }
        glEnd();
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) { glVertex3f(1.7f, 0.2f, 0.62f); glVertex3f(1.7f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.62f); }
        glEnd();

        glPushMatrix(); glTranslatef(0.6f, 0.2f, 0.6f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.6f, 0.2f, 0.2f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(1.7f, 0.2f, 0.2f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(1.7f, 0.2f, 0.6f); drawTire(0.09f, 0.05f); glPopMatrix();
    }

private:
    void drawTire(float radius, float thickness) {
        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 20; i++) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            glVertex3f(radius * cos(angle), radius * sin(angle), thickness / 2.0f);
            glVertex3f(radius * cos(angle), radius * sin(angle), -thickness / 2.0f);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, thickness / 2.0f);
        for (int i = 0; i <= 20; i++) glVertex3f(radius * cos(i * 3.14159f * 2.0f / 20.0f), radius * sin(i * 3.14159f * 2.0f / 20.0f), thickness / 2.0f);
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, -thickness / 2.0f);
        for (int i = 20; i >= 0; i--) glVertex3f(radius * cos(i * 3.14159f * 2.0f / 20.0f), radius * sin(i * 3.14159f * 2.0f / 20.0f), -thickness / 2.0f);
        glEnd();
    }

    void drawExhaust(float radius1, float radius2, float length) {
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 10; i++) {
            float angle = i * 3.14159f * 2.0f / 10.0f;
            glVertex3f(radius1 * cos(angle), radius1 * sin(angle), 0.0f);
            glVertex3f(radius2 * cos(angle), radius2 * sin(angle), length);
        }
        glEnd();
    }
};

#endif // CAR_H