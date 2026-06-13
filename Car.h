#ifndef CAR_H
#define CAR_H

#include <GLFW/glfw3.h>
#include <math.h>
#include <time.h>

class Car {
private:
    void drawQuad(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) {
        float nx = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
        float ny = (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1);
        float nz = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.0001f) { nx /= len; ny /= len; nz /= len; }

        glBegin(GL_QUADS);
        glNormal3f(nx, ny, nz);
        glVertex3f(x1, y1, z1); glVertex3f(x2, y2, z2); glVertex3f(x3, y3, z3); glVertex3f(x4, y4, z4);
        glEnd();
    }

    void drawTri(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
        float nx = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
        float ny = (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1);
        float nz = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.0001f) { nx /= len; ny /= len; nz /= len; }

        glBegin(GL_TRIANGLES);
        glNormal3f(nx, ny, nz);
        glVertex3f(x1, y1, z1); glVertex3f(x2, y2, z2); glVertex3f(x3, y3, z3);
        glEnd();
    }

    void drawTire(float radius, float thickness) {
        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 20; i++) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            float cx = radius * cos(angle);
            float cy = radius * sin(angle);
            glVertex3f(cx, cy, thickness / 2.0f);
            glVertex3f(cx, cy, -thickness / 2.0f);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, thickness / 2.0f);
        for (int i = 0; i <= 20; i++) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            glVertex3f(radius * cos(angle), radius * sin(angle), thickness / 2.0f);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, -thickness / 2.0f);
        for (int i = 20; i >= 0; i--) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            glVertex3f(radius * cos(angle), radius * sin(angle), -thickness / 2.0f);
        }
        glEnd();
    }

    void drawExhaust(float radius1, float radius2, float length) {
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 10; i++) {
            float angle = i * 3.14159f * 2.0f / 10.0f;
            float c = cos(angle);
            float s = sin(angle);
            glVertex3f(radius1 * c, radius1 * s, 0.0f);
            glVertex3f(radius2 * c, radius2 * s, length);
        }
        glEnd();
    }

public:
    float x, y, z;
    float wheelAngle;
    float r, g, b;

    float colorR = 1.0f;
    float colorG = 1.0f;
    float colorB = 1.0f;

    int indicatorMode = 0;

    Car(float startX = 0.0f, float startY = 0.0f, float startZ = 0.0f)
        : x(startX), y(startY), z(startZ), wheelAngle(0.0f), r(0.0f), g(0.7f), b(0.15f) {
    }

    void toggleLeftIndicator() { indicatorMode = (indicatorMode == 1) ? 0 : 1; }
    void toggleRightIndicator() { indicatorMode = (indicatorMode == 2) ? 0 : 2; }
    void toggleHazardLights() { indicatorMode = (indicatorMode == 3) ? 0 : 3; }

    void move(float speed) {
        x += speed;
        wheelAngle += speed * 50.0f;
    }

    void setColor(float red, float green, float blue) {
        r = red; g = green; b = blue;
    }

    void draw_model_only() {
        GLfloat body_specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
        GLfloat body_shininess[] = { 50.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, body_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, body_shininess);

        // ==========================================
        // 1. GŁÓWNA BRYŁA SAMOCHODU
        // ==========================================
        glColor3f(r, g, b);
        drawQuad(0.2f, 0.4f, 0.6f, 0.6f, 0.5f, 0.6f, 0.6f, 0.5f, 0.2f, 0.2f, 0.4f, 0.2f);
        drawQuad(0.2f, 0.4f, 0.6f, 0.6f, 0.2f, 0.6f, 0.6f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f);
        drawQuad(0.2f, 0.2f, 0.6f, 0.2f, 0.4f, 0.6f, 0.2f, 0.4f, 0.2f, 0.2f, 0.2f, 0.2f);
        drawQuad(0.6f, 0.2f, 0.6f, 0.6f, 0.5f, 0.6f, 0.6f, 0.5f, 0.2f, 0.6f, 0.2f, 0.2f);
        drawQuad(0.2f, 0.2f, 0.6f, 0.6f, 0.2f, 0.6f, 0.6f, 0.5f, 0.6f, 0.2f, 0.4f, 0.6f);
        drawQuad(0.2f, 0.2f, 0.2f, 0.6f, 0.2f, 0.2f, 0.6f, 0.5f, 0.2f, 0.2f, 0.4f, 0.2f);
        drawQuad(0.7f, 0.65f, 0.6f, 0.7f, 0.65f, 0.2f, 1.7f, 0.65f, 0.2f, 1.7f, 0.65f, 0.6f);

        drawQuad(1.8f, 0.5f, 0.6f, 1.8f, 0.5f, 0.2f, 2.1f, 0.4f, 0.2f, 2.1f, 0.4f, 0.6f);
        drawQuad(2.1f, 0.2f, 0.6f, 2.1f, 0.2f, 0.2f, 1.8f, 0.2f, 0.6f, 1.8f, 0.2f, 0.6f);
        drawQuad(2.1f, 0.4f, 0.6f, 2.1f, 0.4f, 0.2f, 2.1f, 0.2f, 0.2f, 2.1f, 0.2f, 0.6f);
        drawQuad(1.8f, 0.2f, 0.2f, 1.8f, 0.5f, 0.2f, 2.1f, 0.4f, 0.2f, 2.1f, 0.2f, 0.2f);
        drawQuad(1.8f, 0.2f, 0.6f, 1.8f, 0.5f, 0.6f, 2.1f, 0.4f, 0.6f, 2.1f, 0.2f, 0.6f);

        drawQuad(0.6f, 0.5f, 0.6f, 0.6f, 0.2f, 0.6f, 1.8f, 0.2f, 0.6f, 1.8f, 0.5f, 0.6f);
        drawQuad(0.6f, 0.2f, 0.6f, 0.6f, 0.2f, 0.2f, 1.8f, 0.2f, 0.2f, 1.8f, 0.2f, 0.6f);
        drawQuad(0.6f, 0.5f, 0.2f, 0.6f, 0.2f, 0.2f, 1.8f, 0.2f, 0.2f, 1.8f, 0.5f, 0.2f);

        drawQuad(0.7f, 0.65f, 0.2f, 0.7f, 0.5f, 0.2f, 0.75f, 0.5f, 0.2f, 0.77f, 0.65f, 0.2f);
        drawQuad(1.2f, 0.65f, 0.2f, 1.2f, 0.5f, 0.2f, 1.25f, 0.5f, 0.2f, 1.27f, 0.65f, 0.2f);
        drawQuad(1.65f, 0.65f, 0.2f, 1.65f, 0.5f, 0.2f, 1.7f, 0.5f, 0.2f, 1.7f, 0.65f, 0.2f);
        drawQuad(0.75f, 0.65f, 0.2f, 0.75f, 0.63f, 0.2f, 1.7f, 0.63f, 0.2f, 1.7f, 0.65f, 0.2f);
        drawQuad(0.75f, 0.65f, 0.6f, 0.75f, 0.63f, 0.6f, 1.7f, 0.63f, 0.6f, 1.7f, 0.65f, 0.6f);

        // ==========================================
        // 2. SZYBY SAMOCHODU
        // ==========================================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        GLfloat glass_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat glass_shininess[] = { 120.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, glass_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, glass_shininess);
        glColor4f(0.5f, 0.8f, 1.0f, 0.4f);

        drawQuad(0.77f, 0.63f, 0.2f, 0.75f, 0.5f, 0.2f, 1.2f, 0.5f, 0.2f, 1.22f, 0.63f, 0.2f);
        drawQuad(1.27f, 0.63f, 0.2f, 1.25f, 0.5f, 0.2f, 1.65f, 0.5f, 0.2f, 1.67f, 0.63f, 0.2f);
        drawQuad(0.77f, 0.63f, 0.6f, 0.75f, 0.5f, 0.6f, 1.2f, 0.5f, 0.6f, 1.22f, 0.63f, 0.6f);
        drawQuad(1.27f, 0.63f, 0.6f, 1.25f, 0.5f, 0.6f, 1.65f, 0.5f, 0.6f, 1.67f, 0.63f, 0.6f);
        drawQuad(0.6f, 0.5f, 0.6f, 0.6f, 0.5f, 0.2f, 0.7f, 0.65f, 0.2f, 0.7f, 0.65f, 0.6f);
        drawQuad(1.7f, 0.65f, 0.6f, 1.7f, 0.65f, 0.2f, 1.8f, 0.5f, 0.2f, 1.8f, 0.5f, 0.6f);

        drawTri(0.6f, 0.5f, 0.6f, 0.7f, 0.65f, 0.6f, 0.7f, 0.5f, 0.6f);
        drawTri(0.6f, 0.5f, 0.2f, 0.7f, 0.65f, 0.2f, 0.7f, 0.5f, 0.2f);
        drawTri(1.7f, 0.65f, 0.2f, 1.8f, 0.5f, 0.2f, 1.7f, 0.5f, 0.2f);
        drawTri(1.7f, 0.65f, 0.6f, 1.8f, 0.5f, 0.6f, 1.7f, 0.5f, 0.6f);
        glDisable(GL_BLEND);

        glMaterialfv(GL_FRONT, GL_SPECULAR, body_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, body_shininess);

        // ZEGAR MIGANIE
        bool flashOn = ((int)(clock() / 400) % 2) == 0;
        bool showLeft = flashOn && (indicatorMode == 1 || indicatorMode == 3);
        bool showRight = flashOn && (indicatorMode == 2 || indicatorMode == 3);

        // ==========================================
        // 3. MATRYCA ŚWIATEŁ NA KAROSERII
        // ==========================================
        GLfloat noGlow[] = { 0.0f, 0.0f, 0.0f, 1.0f };

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

        // ==========================================
        // 4. POŚWIATY PODŁOŻOWE
        // ==========================================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);

        float roadY = 0.015f;
        glBegin(GL_QUADS);
        float glowStart = 2.1f; float glowEnd = 2.8f;

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
            glColor4f(1.0f, 0.5f, 0.0f, 0.25f); glVertex3f(-0.05f, roadY, 0.53f);  glVertex3f(-0.05f, roadY, 0.61f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.0f);  glVertex3f(-0.75f, roadY, 0.61f); glVertex3f(-0.75f, roadY, 0.53f);

            glColor4f(1.0f, 0.5f, 0.0f, 0.25f); glVertex3f(2.1f, roadY, 0.53f);  glVertex3f(2.1f, roadY, 0.61f);
            glColor4f(1.0f, 0.5f, 0.0f, 0.0f);  glVertex3f(2.7f, roadY, 0.61f);  glVertex3f(2.7f, roadY, 0.53f);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);

        // ==========================================
        // 5. WYDECH
        // ==========================================
        glPushMatrix();
        glColor3f(0.4f, 0.4f, 0.4f);
        glTranslatef(1.65f, 0.2f, 0.3f);
        glRotatef(90.0f, 0, 1, 0);
        drawExhaust(0.02f, 0.03f, 0.5f);
        glPopMatrix();

        // ==========================================
        // 6. KOŁA
        // ==========================================
        glColor3f(0.7f, 0.7f, 0.7f);
        glPushMatrix();
        float theta;

        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(0.6f, 0.2f, 0.62f);
            glVertex3f(0.6f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.62f);
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(0.6f, 0.2f, 0.18f);
            glVertex3f(0.6f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.18f);
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(1.7f, 0.2f, 0.18f);
            glVertex3f(1.7f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.18f);
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(1.7f, 0.2f, 0.62f);
            glVertex3f(1.7f + (0.08f * (cos(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.2f + (0.08f * (sin(((theta + wheelAngle) * 3.14f) / 180.0f))), 0.62f);
        }
        glEnd();

        glPushMatrix(); glTranslatef(0.6f, 0.2f, 0.6f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.6f, 0.2f, 0.2f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(1.7f, 0.2f, 0.2f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(1.7f, 0.2f, 0.6f); drawTire(0.09f, 0.05f); glPopMatrix();
        glPopMatrix();
    }

    void draw() {
        glPushMatrix();
        glTranslatef(x, y, z);
        draw_model_only();
        glPopMatrix();
    }

private:
    void getLog() {}
};

#endif // CAR_H