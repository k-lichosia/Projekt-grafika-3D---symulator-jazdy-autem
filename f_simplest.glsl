#version 330 core

// =======================================================================
// --- DANE WEJSCIOWE I WYJSCIOWE ---
// =======================================================================
in vec3 vNormal;  // Wektor normalny z Vertex Shadera
in vec3 fragPos;  // Pozycja piksela w przestrzeni swiata

out vec4 FragColor; // Ostateczny kolor piksela na ekranie

// =======================================================================
// --- UNIFORMY (ZMIENNE GLOBALNE) ---
// =======================================================================
uniform vec3 objectColor;        // Bazowy kolor obiektu (np. karoserii aut NPC)
uniform vec3 lightPositions[10]; // Pozycje latarni miejskich
uniform int lightCount;          // Aktualna liczba swiecacych latarni

void main() {
    // Upewniamy sie, ze wektor normalny ma dlugosc 1
    vec3 normal = normalize(vNormal);
    
    // 1. Swiatlo otoczenia (Ambient) - minimalne widoczne oswietlenie w mroku
    vec3 ambient = 0.25 * objectColor; 
    
    // Zmienna do kumulowania swiatla ze wszystkich zrodel
    vec3 diffuseAccumulator = vec3(0.0);

    // =======================================================================
    // --- 2. OBLICZANIE WPLYWU LATARNI (PETLA SWIATLA) ---
    // =======================================================================
    for(int i = 0; i < lightCount; i++) {
        // Obliczanie kierunku padania swiatla
        vec3 lightDir = normalize(lightPositions[i] - fragPos);
        
        // Sila oswietlenia zalezna od kata (Diffuse)
        float diff = max(dot(normal, lightDir), 0.0);
        
        // Dystans od latarni do piksela
        float distance = length(lightPositions[i] - fragPos);
        
        // Tlumienie swiatla wraz z odlegloscia (Attenuation)
        float attenuation = 1.0 / (1.0 + 0.05 * distance + 0.005 * (distance * distance));
        
        // Dodajemy cieple, zolte swiatlo latarni przemnozone przez tlumienie
        diffuseAccumulator += diff * vec3(0.8, 0.75, 0.55) * attenuation;
    }

    // =======================================================================
    // --- 3. OSTATECZNY KOLOR PIKSELA ---
    // =======================================================================
    // Mieszamy swiatlo otoczenia ze swiatlem latarni padajacym na kolor obiektu
    vec3 finalColor = ambient + (diffuseAccumulator * objectColor);
    
    // Funkcja min() zabezpiecza przed "przepaleniem" (utrzymuje wartosci max do 1.0)
    FragColor = vec4(min(finalColor, vec3(1.0)), 1.0);
}