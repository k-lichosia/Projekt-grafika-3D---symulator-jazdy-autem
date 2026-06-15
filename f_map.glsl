#version 330 core

// Dane wejsciowe z Vertex Shadera (musza idealnie pasowac do v_map.glsl)
in vec4 vColor;
in vec4 fragPos;
in vec2 iTexCoord;
in vec3 localPos;
in float Height; // <-- Odbieramy wysokosc warstwy (0.0 do 1.0)

out vec4 pixelColor;

// Dane o swiatlach
uniform vec3 lightPositions[10];
uniform int lightCount;

// Obsługa tekstury
uniform sampler2D tex;
uniform int useTexture;

// --- UNIFORMY DO OBSLUGI WARSTW ---
uniform int currentLayer;
uniform int totalLayers;

uniform int isCrashedStatus;

void main() {
    vec3 normal = normalize(cross(dFdx(fragPos.xyz), dFdy(fragPos.xyz)));
    vec3 baseColor;
    float alpha = 1.0;

    if (useTexture == 1) {
        // Zwykle tekstrurowanie (np. chodnik z iTexCoord)
        vec4 texColor = texture(tex, iTexCoord);
        baseColor = texColor.rgb * 0.7;
        alpha = texColor.a;
    }
    else if (useTexture == 2) {
        // Mapowanie UV dla tekstury asfaltu
        vec2 autoUV = localPos.xz * 0.4;
        vec4 texColor = texture(tex, autoUV);
        
        // Wykrywamy podbudowe asfaltu (ciemne wierzcholki)
        vec3 adjustedBase;
        if (vColor.r < 0.2 && vColor.g < 0.2) {
            adjustedBase = vec3(0.42, 0.43, 0.45);
        } else {
            adjustedBase = vColor.rgb;
        }
        
        float grain = (texColor.r + texColor.g + texColor.b) / 3.0;
        float textureImpact = pow(grain, 1.1) * 1.3;
        baseColor = adjustedBase * textureImpact * 0.75;
        alpha = vColor.a;
    }
    else if (useTexture == 3) {
        if (abs(normal.y) > 0.7) {
            baseColor = vec3(0.2, 0.2, 0.22); // Dach
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
        // Skalujemy teksture na bazie pozycji swiata, by kepki byly drobne i geste
        vec2 grassUV = localPos.xz * 2.5;
        vec4 texColor = texture(tex, grassUV);
        
        // Soczysty, jasny kolor trawy dopasowany do sceny nocnej
        baseColor = texColor.rgb * 0.6;
        alpha = 1.0;
    }
    else if (useTexture == 5) {
        float u = atan(localPos.z, localPos.x) / (2.0 * 3.14159) + 0.5;
        float v = localPos.y + 0.5;

        float scaledU = u * 4.0;
        float wrapU = fract(scaledU); 
        
        vec3 color1 = textureLod(tex, vec2(wrapU, v), 0.0).rgb;
        vec3 color2 = textureLod(tex, vec2(fract(wrapU + 0.5), v), 0.0).rgb;
        
        float distToSeam = abs(wrapU - 0.5) * 2.0;
        float blend = smoothstep(0.4, 0.6, distToSeam);
        
        baseColor = mix(color1, color2, blend);

        if (isCrashedStatus == 1) {
            // Mieszamy oryginalne gwiazdy z mocnym czerwonym kolorem (70% czerwieni, 30% gwiazd)
            baseColor = mix(baseColor, vec3(0.9, 0.1, 0.1), 0.3);
        }

        alpha = 1.0;
    }
    else {
        // Brak tekstury (czysty kolor wierzcholkow)
        baseColor = vColor.rgb * 0.8;
        alpha = vColor.a;
    }

    // Ambient - delikatne swiatlo stale sceny
    vec3 ambient = 0.55 * baseColor;
    vec3 diffuseAccumulator = vec3(0.0);

    // Petla obliczajaca swiatlo od kazdej latarni miejskiej
    for(int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fragPos.xyz);
        float diff = max(dot(normal, lightDir), 0.0);
        
        float distance = length(lightPositions[i] - fragPos.xyz);
        float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.001 * (distance * distance));
        
        diffuseAccumulator += diff * vec3(1.0, 0.9, 0.6) * attenuation;
    }

    // Ostateczne wyznaczenie koloru piksela
    if (useTexture == 5){
        pixelColor = vec4(baseColor, alpha);
    }
    else if (vColor.r > 0.9 && vColor.g > 0.8 && useTexture == 0) {
        pixelColor = vec4(vColor.rgb, vColor.a);
    } else {
        // Dla trawy mieszamy swiatlo latarni z rozjasniona baza warstwowa
        pixelColor = vec4((ambient + diffuseAccumulator) * baseColor, alpha);
    }
}