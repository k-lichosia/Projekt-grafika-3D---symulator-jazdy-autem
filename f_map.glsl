#version 330

in vec4 vColor;
in vec4 fragPos;
in vec2 iTexCoord;      // 1. Odbieramy wspó³rzêdne tekstury z Vertex Shadera

out vec4 pixelColor;

// Dane o œwiat³ach
uniform vec3 lightPositions[10];
uniform int lightCount;

// 2. NOWE: Obs³uga tekstury
uniform sampler2D tex;  // Sampler dla pliku chodnik.jpg
uniform int useTexture; // Prze³¹cznik: 1 = tekstura, 0 = kolor vColor

void main() {
    // Obliczamy wektor normalny
    vec3 normal = normalize(cross(dFdx(fragPos.xyz), dFdy(fragPos.xyz)));

    // 3. Wybieramy bazowy kolor: albo ze zdjêcia, albo z vColor
    vec3 baseColor;
    float alpha;
    if (useTexture == 1) {
        vec4 texColor = texture(tex, iTexCoord);
        baseColor = texColor.rgb;
        alpha = texColor.a;
    } else {
        baseColor = vColor.rgb;
        alpha = vColor.a;
    }

    // Ambient - delikatne œwiat³o sta³e
    vec3 ambient = 0.55 * baseColor; 
    
    vec3 diffuseAccumulator = vec3(0.0);

    // Pêtla obliczaj¹ca œwiat³o od ka¿dej latarni
    for(int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fragPos.xyz);
        float diff = max(dot(normal, lightDir), 0.0);
        
        float distance = length(lightPositions[i] - fragPos.xyz);
        float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.001 * (distance * distance));
        
        // Dodajemy blask latarni
        diffuseAccumulator += diff * vec3(1.0, 0.9, 0.6) * attenuation;
    }

    // Finalny kolor
    // Jeœli to ¿arówka (bardzo jasny ¿ó³ty), œwieci w³asnym œwiat³em
    if (vColor.r > 0.9 && vColor.g > 0.8 && useTexture == 0) {
        pixelColor = vColor;
    } else {
        // Dla reszty œwiata: (Ambient + Suma œwiate³) * Kolor bazy (tekstura lub vColor)
        pixelColor = vec4((ambient + diffuseAccumulator) * baseColor, alpha);
    }
}