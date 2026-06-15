#version 330 core

// =======================================================================
// --- ZMIENNE WEJSCIOWE (Z BUFOROW VBO) ---
// =======================================================================
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

// =======================================================================
// --- ZMIENNE WYJSCIOWE (DO FRAGMENT SHADERA) ---
// =======================================================================
out vec4 vColor;
out vec4 fragPos; 
out vec2 iTexCoord;
out vec3 localPos; 
out float Height; // Wysokosc warstwy dla 3D trawy (od 0.0 do 1.0)

// =======================================================================
// --- UNIFORMY (MACIERZE I PARAMETRY) ---
// =======================================================================
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

// Parametry warstwowej trawy (Shell Texturing)
uniform int currentLayer;   // Numer aktualnie rysowanej warstwy
uniform int totalLayers;    // Maksymalna liczba warstw
uniform float grassHeight;  // Calkowita wysokosc kepki trawy

void main() {
    // 1. Zapisujemy czysta, poczatkowa pozycje lokalna do teksturowania
    localPos = vertex.xyz;
    vColor = color;
    iTexCoord = texCoord;
    
    // 2. Kopiujemy wierzcholek, aby moc go zmodyfikowac (podniesc)
    vec4 displacedPos = vertex; 
    
    // 3. Obliczamy wysokosc warstwy i przesuwamy wierzcholek do gory (dla trawy)
    if (totalLayers > 0) {
        Height = float(currentLayer) / float(totalLayers);
        displacedPos.y += Height * grassHeight;
    } else {
        Height = 0.0;
    }
    
    // 4. Obliczamy pozycje wierzcholka w przestrzeni swiata (po przesunieciu)
    fragPos = M * displacedPos; 
    
    // 5. Ostateczna, przeliczona pozycja wierzcholka na ekranie
    gl_Position = P * V * M * displacedPos;
}