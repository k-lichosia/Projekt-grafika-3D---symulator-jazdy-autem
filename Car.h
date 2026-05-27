#ifndef CAR_H
#define CAR_H

#include <GLFW/glfw3.h>
#include <math.h>

class Car {
private:
    // Pomocnicza funkcja do rysowania opony (zamiast glutSolidTorus)
    void drawTire(float radius, float thickness) {
        glColor3f(0.05f, 0.05f, 0.05f); // Ciemnoszary/czarny kolor opony

        // Bie¿nik (obramowanie opony)
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 20; i++) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            float cx = radius * cos(angle);
            float cy = radius * sin(angle);
            glVertex3f(cx, cy, thickness / 2.0f);
            glVertex3f(cx, cy, -thickness / 2.0f);
        }
        glEnd();

        // Felga/Zewnêtrzna strona opony
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, thickness / 2.0f);
        for (int i = 0; i <= 20; i++) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            glVertex3f(radius * cos(angle), radius * sin(angle), thickness / 2.0f);
        }
        glEnd();

        // Wewnêtrzna strona opony
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, -thickness / 2.0f);
        for (int i = 20; i >= 0; i--) {
            float angle = i * 3.14159f * 2.0f / 20.0f;
            glVertex3f(radius * cos(angle), radius * sin(angle), -thickness / 2.0f);
        }
        glEnd();
    }

    // Pomocnicza funkcja do rysowania rury wydechowej (zamiast gluCylinder)
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
    float x, y, z;        // Pozycja samochodu
    float wheelAngle;     // K¹t obrotu kó³
    float r, g, b;        // Kolor karoserii

    // Konstruktor ustawiaj¹cy domyœln¹ pozycjê
    Car(float startX = 0.0f, float startY = 0.0f, float startZ = 0.0f)
        : x(startX), y(startY), z(startZ), wheelAngle(0.0f), r(0.8f), g(0.0f), b(0.0f) {
    }

    // Funkcja aktualizuj¹ca pozycjê auta i obrót kó³
    void move(float speed) {
        x += speed; // Przesuniêcie wzd³u¿ osi X
        wheelAngle += speed * 50.0f; // Animacja obrotu kó³
    }

    // Funkcja ustawiaj¹ca nowy kolor
    void setColor(float red, float green, float blue) {
        r = red; g = green; b = blue;
    }

    // FUNKCJA RYSOWAÑCA (BEZ ZMIANY POZYCJI) - idealna do drawScene
    void draw_model_only() {
        // ==========================================
        // 1. G£ÓWNA BRY£A SAMOCHODU (QUADS)
        // ==========================================
        glBegin(GL_QUADS);

        // --- FRONT BODY ---
        glColor3f(r, g, b);
        glVertex3f(0.2, 0.4, 0.6); glVertex3f(0.6, 0.5, 0.6); glVertex3f(0.6, 0.5, 0.2); glVertex3f(0.2, 0.4, 0.2);
        glVertex3f(0.2, 0.4, 0.6); glVertex3f(0.6, 0.2, 0.6); glVertex3f(0.6, 0.2, 0.2); glVertex3f(0.2, 0.2, 0.2);
        glVertex3f(0.2, 0.2, 0.6); glVertex3f(0.2, 0.4, 0.6); glVertex3f(0.2, 0.4, 0.2); glVertex3f(0.2, 0.2, 0.2);
        glVertex3f(0.6, 0.2, 0.6); glVertex3f(0.6, 0.5, 0.6); glVertex3f(0.6, 0.5, 0.2); glVertex3f(0.6, 0.2, 0.2);
        glVertex3f(0.2, 0.2, 0.6); glVertex3f(0.6, 0.2, 0.6); glVertex3f(0.6, 0.5, 0.6); glVertex3f(0.2, 0.4, 0.6);
        glVertex3f(0.2, 0.2, 0.2); glVertex3f(0.6, 0.2, 0.2); glVertex3f(0.6, 0.5, 0.2); glVertex3f(0.2, 0.4, 0.2);
        glVertex3f(0.7, 0.65, 0.6); glVertex3f(0.7, 0.65, 0.2); glVertex3f(1.7, 0.65, 0.2); glVertex3f(1.7, 0.65, 0.6);

        // --- BACK GUARD ---
        glColor3f(r, g, b);
        glVertex3f(1.8, 0.5, 0.6); glVertex3f(1.8, 0.5, 0.2); glVertex3f(2.1, 0.4, 0.2); glVertex3f(2.1, 0.4, 0.6);
        glVertex3f(2.1, 0.2, 0.6); glVertex3f(2.1, 0.2, 0.2); glVertex3f(1.8, 0.2, 0.6); glVertex3f(1.8, 0.2, 0.6);
        glVertex3f(2.1, 0.4, 0.6); glVertex3f(2.1, 0.4, 0.2); glVertex3f(2.1, 0.2, 0.2); glVertex3f(2.1, 0.2, 0.6);
        glVertex3f(1.8, 0.2, 0.2); glVertex3f(1.8, 0.5, 0.2); glVertex3f(2.1, 0.4, 0.2); glVertex3f(2.1, 0.2, 0.2);
        glVertex3f(1.8, 0.2, 0.6); glVertex3f(1.8, 0.5, 0.6); glVertex3f(2.1, 0.4, 0.6); glVertex3f(2.1, 0.2, 0.6);

        // --- MIDDLE BODY ---
        glVertex3f(0.6, 0.5, 0.6); glVertex3f(0.6, 0.2, 0.6); glVertex3f(1.8, 0.2, 0.6); glVertex3f(1.8, 0.5, 0.6);
        glVertex3f(0.6, 0.2, 0.6); glVertex3f(0.6, 0.2, 0.2); glVertex3f(1.8, 0.2, 0.2); glVertex3f(1.8, 0.2, 0.6);
        glVertex3f(0.6, 0.5, 0.2); glVertex3f(0.6, 0.2, 0.2); glVertex3f(1.8, 0.2, 0.2); glVertex3f(1.8, 0.5, 0.2);

        // --- ENTER WINDOW ---
        glColor3f(0.3, 0.3, 0.3);
        glVertex3f(0.77, 0.63, 0.2); glVertex3f(0.75, 0.5, 0.2); glVertex3f(1.2, 0.5, 0.2); glVertex3f(1.22, 0.63, 0.2);
        glVertex3f(1.27, 0.63, 0.2); glVertex3f(1.25, 0.5, 0.2); glVertex3f(1.65, 0.5, 0.2); glVertex3f(1.67, 0.63, 0.2);

        glColor3f(r, g, b);
        glVertex3f(0.7, 0.65, 0.2); glVertex3f(0.7, 0.5, 0.2); glVertex3f(0.75, 0.5, 0.2); glVertex3f(0.77, 0.65, 0.2);
        glVertex3f(1.2, 0.65, 0.2); glVertex3f(1.2, 0.5, 0.2); glVertex3f(1.25, 0.5, 0.2); glVertex3f(1.27, 0.65, 0.2);
        glVertex3f(1.65, 0.65, 0.2); glVertex3f(1.65, 0.5, 0.2); glVertex3f(1.7, 0.5, 0.2); glVertex3f(1.7, 0.65, 0.2);
        glVertex3f(0.75, 0.65, 0.2); glVertex3f(0.75, 0.63, 0.2); glVertex3f(1.7, 0.63, 0.2); glVertex3f(1.7, 0.65, 0.2);
        glVertex3f(0.75, 0.65, 0.6); glVertex3f(0.75, 0.63, 0.6); glVertex3f(1.7, 0.63, 0.6); glVertex3f(1.7, 0.65, 0.6);

        glColor3f(0.3, 0.3, 0.3);
        glVertex3f(0.77, 0.63, 0.6); glVertex3f(0.75, 0.5, 0.6); glVertex3f(1.2, 0.5, 0.6); glVertex3f(1.22, 0.63, 0.6);
        glVertex3f(1.27, 0.63, 0.6); glVertex3f(1.25, 0.5, 0.6); glVertex3f(1.65, 0.5, 0.6); glVertex3f(1.67, 0.63, 0.6);

        glColor3f(r, g, b);
        glVertex3f(0.7, 0.65, 0.6); glVertex3f(0.7, 0.5, 0.6); glVertex3f(0.75, 0.5, 0.6); glVertex3f(0.77, 0.65, 0.6);
        glVertex3f(1.2, 0.65, 0.6); glVertex3f(1.2, 0.5, 0.6); glVertex3f(1.25, 0.5, 0.6); glVertex3f(1.27, 0.65, 0.6);
        glVertex3f(1.65, 0.65, 0.6); glVertex3f(1.65, 0.5, 0.6); glVertex3f(1.7, 0.5, 0.6); glVertex3f(1.7, 0.65, 0.6);

        glColor3f(0.3, 0.3, 0.3);
        glVertex3f(0.6, 0.5, 0.6); glVertex3f(0.6, 0.5, 0.2); glVertex3f(0.7, 0.65, 0.2); glVertex3f(0.7, 0.65, 0.6);
        glVertex3f(1.7, 0.65, 0.6); glVertex3f(1.7, 0.65, 0.2); glVertex3f(1.8, 0.5, 0.2); glVertex3f(1.8, 0.5, 0.6);
        glEnd();

        // ==========================================
        // 2. DETALE (TRÓJK¥TY)
        // ==========================================
        glBegin(GL_TRIANGLES);
        glColor3f(0.3, 0.3, 0.3);

        glVertex3f(0.6, 0.5, 0.6); glVertex3f(0.7, 0.65, 0.6); glVertex3f(0.7, 0.5, 0.6);
        glVertex3f(0.6, 0.5, 0.2); glVertex3f(0.7, 0.65, 0.2); glVertex3f(0.7, 0.5, 0.2);
        glVertex3f(1.7, 0.65, 0.2); glVertex3f(1.8, 0.5, 0.2); glVertex3f(1.7, 0.5, 0.2);
        glVertex3f(1.7, 0.65, 0.6); glVertex3f(1.8, 0.5, 0.6); glVertex3f(1.7, 0.5, 0.6);
        glEnd();

        // ==========================================
        // 3. SYSTEM ZAP£ONU (Rura z ty³u)
        // ==========================================
        glPushMatrix();
        glColor3f(0.3, 0.3, 0.7);
        glTranslatef(1.65, 0.2, 0.3);
        glRotatef(90.0, 0, 1, 0);
        drawExhaust(0.02f, 0.03f, 0.5f); // Nowa funkcja bez glu!
        glPopMatrix();

        // ==========================================
        // 4. KO£A (WHEELS)
        // ==========================================
        glColor3f(0.7, 0.7, 0.7);
        glPushMatrix();

        float theta;

        // Szprychy ko³a 1
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(0.6, 0.2, 0.62);
            glVertex3f(0.6 + (0.08 * (cos(((theta + wheelAngle) * 3.14) / 180))), 0.2 + (0.08 * (sin(((theta + wheelAngle) * 3.14) / 180))), 0.62);
        }
        glEnd();

        // Szprychy ko³a 2
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(0.6, 0.2, 0.18);
            glVertex3f(0.6 + (0.08 * (cos(((theta + wheelAngle) * 3.14) / 180))), 0.2 + (0.08 * (sin(((theta + wheelAngle) * 3.14) / 180))), 0.18);
        }
        glEnd();

        // Szprychy ko³a 3
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(1.7, 0.2, 0.18);
            glVertex3f(1.7 + (0.08 * (cos(((theta + wheelAngle) * 3.14) / 180))), 0.2 + (0.08 * (sin(((theta + wheelAngle) * 3.14) / 180))), 0.18);
        }
        glEnd();

        // Szprychy ko³a 4
        glBegin(GL_LINE_STRIP);
        for (theta = 0; theta < 360; theta = theta + 40) {
            glVertex3f(1.7, 0.2, 0.62);
            glVertex3f(1.7 + (0.08 * (cos(((theta + wheelAngle) * 3.14) / 180))), 0.2 + (0.08 * (sin(((theta + wheelAngle) * 3.14) / 180))), 0.62);
        }
        glEnd();

        // Opony - Narysowane rêcznie bez GLUTa!
        glPushMatrix();
        glTranslatef(0.6, 0.2, 0.6);
        drawTire(0.09f, 0.05f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.6, 0.2, 0.2);
        drawTire(0.09f, 0.05f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1.7, 0.2, 0.2);
        drawTire(0.09f, 0.05f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1.7, 0.2, 0.6);
        drawTire(0.09f, 0.05f);
        glPopMatrix();

        glPopMatrix(); // Koniec transformacji kó³
    }

    // Klasyczna funkcja draw - najpierw przesuwa auto na pozycje X,Y,Z, a potem wywo³uje model
    void draw() {
        glPushMatrix();
        glTranslatef(x, y, z);
        draw_model_only();
        glPopMatrix();
    }
};

#endif // CAR_H