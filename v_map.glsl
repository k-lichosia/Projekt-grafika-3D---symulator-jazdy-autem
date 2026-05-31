#version 330 core

// Atrybuty wejœciowe (musz¹ pasowaæ do Twojego VBO)
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

// Dane wyjœciowe do Fragment Shadera (f_map.glsl)
out vec4 vColor;
out vec4 fragPos; 
out vec2 iTexCoord;
out vec3 localPos; 
out float Height; // <-- TO BY£O POTRZEBNE! Musimy przekazaæ Height do f_map.glsl

// Macierze transformacji i parametry trawy
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform int currentLayer;   // Numer rysowanej warstwy
uniform int totalLayers;    // Maksymalna liczba warstw (np. 15)
uniform float grassHeight;  // Wysokoœæ ca³ej kêpki (np. 0.25)

void main() {
    // 1. Zapisujemy czyst¹ pozycjê lokaln¹
    localPos = vertex.xyz;
    vColor = color;
    iTexCoord = texCoord;
    
    // 2. Kopiujemy wierzcho³ek wejœciowy, ¿eby móc go zmodyfikowaæ (podnieœæ w górê)
    vec4 displacedPos = vertex; 
    
    // 3. Obliczamy wysokoœæ warstwy (0.0 do 1.0)
    if (totalLayers > 0) {
        Height = float(currentLayer) / float(totalLayers);
        // Podnosimy pionowo wierzcho³ek w osi Y o odpowiedni u³amek wysokoœci
        displacedPos.y += Height * grassHeight;
    } else {
        Height = 0.0;
    }

    localPos = vertex.xyz;
    
    // 4. Obliczamy pozycjê wierzcho³ka w przestrzeni œwiata (ju¿ po przesuniêciu)
    fragPos = M * displacedPos; 
    
    // 5. Ostateczna pozycja na ekranie
    gl_Position = P * V * M * displacedPos;
}