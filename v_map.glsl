#version 330

// Atrybuty wejœciowe (zgodne z Twoim layoutem)
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 color;
layout (location = 2) in vec2 texCoord; // Nowe: pobieramy UV z lokalizacji 2

// Dane wyjœciowe do Fragment Shadera
out vec4 vColor;
out vec4 fragPos; 
out vec2 iTexCoord; // WYPROWADZENIE UV DO FRAGMENT SHADERA

// Macierze transformacji
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
    vColor = color;
    
    // Obliczamy pozycjê wierzcho³ka w "œwiecie" (bez perspektywy kamery)
    // Jest to niezbêdne do obliczenia odleg³oœci od œwiat³a
    fragPos = M * vertex; 
    iTexCoord = texCoord;
    // Standardowe obliczenie pozycji na ekranie
    gl_Position = P * V * M * vertex;
}