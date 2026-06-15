#version 330 core

// Atrybuty wejsciowe (musza pasowac do Twojego VBO)
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

// Dane wyjsciowe do Fragment Shadera (f_map.glsl)
out vec4 vColor;
out vec4 fragPos; 
out vec2 iTexCoord;
out vec3 localPos; 
out float Height; // <-- TO BYLO POTRZEBNE! Musimy przekazac Height do f_map.glsl

// Macierze transformacji i parametry trawy
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform int currentLayer;   // Numer rysowanej warstwy
uniform int totalLayers;    // Maksymalna liczba warstw (np. 15)
uniform float grassHeight;  // Wysokosc calej kepki (np. 0.25)

void main() {
    // 1. Zapisujemy czysta pozycje lokalna
    localPos = vertex.xyz;
    vColor = color;
    iTexCoord = texCoord;
    
    // 2. Kopiujemy wierzcholek wejsciowy, zeby moc go zmodyfikowac (podniesc w gore)
    vec4 displacedPos = vertex; 
    
    // 3. Obliczamy wysokosc warstwy (0.0 do 1.0)
    if (totalLayers > 0) {
        Height = float(currentLayer) / float(totalLayers);
        // Podnosimy pionowo wierzcholek w osi Y o odpowiedni ulamek wysokosci
        displacedPos.y += Height * grassHeight;
    } else {
        Height = 0.0;
    }
    
    // 4. Obliczamy pozycje wierzcholka w przestrzeni swiata (juz po przesunieciu)
    fragPos = M * displacedPos; 
    
    // 5. Ostateczna pozycja na ekranie
    gl_Position = P * V * M * displacedPos;
}