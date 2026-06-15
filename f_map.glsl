#version 330 core

// =======================================================================
// --- DANE WEJSCIOWE I WYJSCIOWE ---
// =======================================================================
in vec4 vColor;
in vec4 fragPos;
in vec2 iTexCoord;
in vec3 localPos;
in float Height; // Wysokosc warstwy (0.0 do 1.0) dla trojwymiarowej trawy

out vec4 pixelColor;

// =======================================================================
// --- UNIFORMY ---
// =======================================================================

// Oswietlenie
uniform vec3 lightPositions[10];
uniform int lightCount;

// Teksturowanie i stan gry
uniform sampler2D tex;
uniform int useTexture;      // ID uzywanej tekstury (1=chodnik, 2=asfalt, 3=budynki, 4=trawa, 5=niebo)
uniform int currentLayer;    // Aktualna warstwa geometrii
uniform int totalLayers;     // Maksymalna liczba warstw
uniform int isCrashedStatus; // Flaga kolizji (1 = gracz mial wypadek)

void main() {
    // Obliczamy wektor normalny na podstawie sasiadujacych pikseli (plaskie cieniowanie)
    vec3 normal = normalize(cross(dFdx(fragPos.xyz), dFdy(fragPos.xyz)));
    
    vec3 baseColor;
    float alpha = 1.0;

    // =======================================================================
    // --- NAKLADANIE TEKSTUR I MATERIALOW ---
    // =======================================================================
    if (useTexture == 1) {
        // 1. Chodnik
        vec4 texColor = texture(tex, iTexCoord);
        baseColor = texColor.rgb * 0.7; // Delikatne przyciemnienie
        alpha = texColor.a;
    }
    else if (useTexture == 2) {
        // 2. Asfalt
        vec2 autoUV = localPos.xz * 0.4;
        vec4 texColor = texture(tex, autoUV);
        
        vec3 adjustedBase;
        if (vColor.r < 0.2 && vColor.g < 0.2) {
            adjustedBase = vec3(0.42, 0.43, 0.45); // Kolor podbudowy
        } else {
            adjustedBase = vColor.rgb;
        }
        
        // Wyliczanie ziarnistosci asfaltu
        float grain = (texColor.r + texColor.g + texColor.b) / 3.0;
        float textureImpact = pow(grain, 1.1) * 1.3;
        baseColor = adjustedBase * textureImpact * 0.75;
        alpha = vColor.a;
    }
    else if (useTexture == 3) {
        // 3. Budynki
        if (abs(normal.y) > 0.7) {
            baseColor = vec3(0.2, 0.2, 0.22); // Plaski, jednolity dach
        }
        else {
            vec2 buildingUV;
            if (abs(normal.x) > 0.5) {
                buildingUV = localPos.zy * 1.8;
            } else {
                buildingUV = localPos.xy * 1.8;
            }
            baseColor = texture(tex, buildingUV).rgb * 0.65;
        }
        alpha = 1.0;
    }
    else if (useTexture == 4) {
        // 4. Trawa (skalowana na podstawie pozycji swiata)
        vec2 grassUV = localPos.xz * 2.5;
        vec4 texColor = texture(tex, grassUV);
        baseColor = texColor.rgb * 0.6;
        alpha = 1.0;
    }
    else if (useTexture == 5) {
        // 5. Niebo (Skybox zawiniety w cylinder wokolo mapy)
        float u = atan(localPos.z, localPos.x) / (2.0 * 3.14159) + 0.5;
        float v = localPos.y + 0.5;

        float scaledU = u * 4.0;
        float wrapU = fract(scaledU); 
        
        // Plynne mieszanie dwoch probek (Texture Blending), aby ukryc szew
        vec3 color1 = textureLod(tex, vec2(wrapU, v), 0.0).rgb;
        vec3 color2 = textureLod(tex, vec2(fract(wrapU + 0.5), v), 0.0).rgb;
        
        float distToSeam = abs(wrapU - 0.5) * 2.0;
        float blend = smoothstep(0.4, 0.6, distToSeam);
        baseColor = mix(color1, color2, blend);

        // Czerwony filtr ekranu przy zderzeniu
        if (isCrashedStatus == 1) {
            baseColor = mix(baseColor, vec3(0.9, 0.1, 0.1), 0.3);
        }
        alpha = 1.0;
    }
    else {
        // Pozostale elementy bez przypisanej tekstury (np. pnie drzew)
        baseColor = vColor.rgb * 0.8;
        alpha = vColor.a;
    }

    // =======================================================================
    // --- OBLICZENIA OSWIETLENIA ---
    // =======================================================================
    vec3 ambient = 0.55 * baseColor; // Bazowe swiatlo otoczenia
    vec3 diffuseAccumulator = vec3(0.0);

    // Sumowanie swiatla od kazdej latarni ulicznej
    for(int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fragPos.xyz);
        float diff = max(dot(normal, lightDir), 0.0);
        
        // Tlumienie swiatla wraz z odlegloscia (Zanikanie)
        float distance = length(lightPositions[i] - fragPos.xyz);
        float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.001 * (distance * distance));
        
        diffuseAccumulator += diff * vec3(1.0, 0.9, 0.6) * attenuation;
    }

    // =======================================================================
    // --- OSTATECZNY KOLOR PIKSELA ---
    // =======================================================================
    if (useTexture == 5) {
        // Niebo nie reaguje na cien ani swiatla latarni
        pixelColor = vec4(baseColor, alpha);
    }
    else if (vColor.r > 0.9 && vColor.g > 0.8 && useTexture == 0) {
        // Bardzo jasne obiekty bez tekstury (jak biale pasy) pelnia role odblaskow
        pixelColor = vec4(vColor.rgb, vColor.a);
    } else {
        // Reszta sceny (trawa, budynki, drogi) miesza sie ze swiatlami
        pixelColor = vec4((ambient + diffuseAccumulator) * baseColor, alpha);
    }
}