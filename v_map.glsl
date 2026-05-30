#version 330

// Atrybuty wejœciowe
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

// Dane wyjœciowe do Fragment Shadera
out vec4 vColor;
out vec4 fragPos; 
out vec2 iTexCoord;
out vec3 localPos; // Ta zmienna przeka¿e pozycjê lokaln¹

// Macierze transformacji
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
    // KROK 1: Zapisujemy czyst¹ pozycjê lokaln¹ ZANIM pomno¿ymy j¹ przez macierz M (modelu)
    localPos = vertex.xyz;

    vColor = color;
    
    // Obliczamy pozycjê wierzcho³ka w przestrzeni œwiata
    fragPos = M * vertex; 
    iTexCoord = texCoord;
    
    // Pozycja na ekranie
    gl_Position = P * V * M * vertex;
}