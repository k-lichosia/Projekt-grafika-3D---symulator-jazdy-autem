#version 330 core

// =======================================================================
// --- DANE WEJSCIOWE ---
// =======================================================================
in vec4 vertex; // Pozycja wierzcholka (lokalna)
in vec4 normal; // Wektor normalny wierzcholka

// =======================================================================
// --- ZMIENNE WYJSCIOWE ---
// =======================================================================
out vec3 vNormal; // Wektor normalny przetransformowany do przestrzeni swiata
out vec3 fragPos; // Pozycja wierzcholka w przestrzeni swiata

// =======================================================================
// --- UNIFORMY (MACIERZE TRANSFORMACJI) ---
// =======================================================================
uniform mat4 P; // Macierz projekcji (Perspektywa kamery)
uniform mat4 V; // Macierz widoku (Pozycja kamery)
uniform mat4 M; // Macierz modelu (Pozycja, rotacja i skala obiektu)

void main() {
    // 1. Transformacja wektora normalnego 
    // Uzywamy mat3(M), aby zignorowac przesuniecia, a zachowac tylko rotacje i skale
    vNormal = normalize(mat3(M) * normal.xyz);
    
    // 2. Obliczenie pozycji wierzcholka w przestrzeni swiata
    vec4 worldPos = M * vertex;
    fragPos = worldPos.xyz;
    
    // 3. Ostateczne przeliczenie pozycji 3D na plaski ekran
    gl_Position = P * V * worldPos;
}