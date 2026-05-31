#version 330 core

// Dane wejœciowe z Vertex Shadera (musz¹ idealnie pasowaæ do v_map.glsl)
in vec4 vColor;
in vec4 fragPos;
in vec2 iTexCoord; 
in vec3 localPos;
in float Height; // <-- DODANE: Odbieramy wysokoœæ warstwy (0.0 do 1.0)

out vec4 pixelColor;

// Dane o œwiat³ach
uniform vec3 lightPositions[10];
uniform int lightCount;

// Obs³uga tekstury
uniform sampler2D tex;  
uniform int useTexture; 

// --- DODANE UNIFORMY DO OBS£UGI WARSTW ---
uniform int currentLayer;   
uniform int totalLayers;    

void main() {
    // Obliczamy wektor normalny powierzchni
    vec3 normal = normalize(cross(dFdx(fragPos.xyz), dFdy(fragPos.xyz)));

    vec3 baseColor;
    float alpha = 1.0;

    if (useTexture == 1) {
        // Zwyk³e tekstrurowanie (np. chodnik z iTexCoord)
        vec4 texColor = texture(tex, iTexCoord);
        baseColor = texColor.rgb;
        alpha = texColor.a;
    } 
    else if (useTexture == 2) {
        // Mapowanie UV dla tekstury asfaltu
        vec2 autoUV = fragPos.xz * 0.4;
        vec4 texColor = texture(tex, autoUV);
        
        // Wykrywamy podbudowê asfaltu (ciemne wierzcho³ki)
        vec3 adjustedBase;
        if (vColor.r < 0.2 && vColor.g < 0.2) {
            adjustedBase = vec3(0.42, 0.43, 0.45); 
        } else {
            adjustedBase = vColor.rgb;
        }
        
        float grain = (texColor.r + texColor.g + texColor.b) / 3.0;
        float textureImpact = pow(grain, 1.1) * 1.3;
        baseColor = adjustedBase * textureImpact;
        alpha = vColor.a;
    }
    else if (useTexture == 3) {
        if (abs(normal.y) > 0.7) {
            baseColor = vec3(0.35, 0.35, 0.37); // Dach
        } 
        else {
            vec2 buildingUV;
            if (abs(normal.x) > 0.5) {
                buildingUV = localPos.zy * 1.8; 
            } else {
                buildingUV = localPos.xy * 1.8; 
            }
            baseColor = texture(tex, buildingUV).rgb;
        }
        alpha = 1.0;
    }
    // ====================================================================
    // --- ZMODYFIKOWANA SEKCJA 4: TRÓJWYMIAROWA TRAWA WARSTWOWA ---
    // ====================================================================
else if (useTexture == 4) {
        // Skalujemy teksturê na bazie pozycji œwiata, by kêpki by³y drobne i gêste
        vec2 grassUV = fragPos.xz * 2.5; 
        vec4 texColor = texture(tex, grassUV);
        
        // Soczysty, jasny kolor trawy dopasowany do sceny nocnej
        baseColor = texColor.rgb * 1.5; 
        alpha = 1.0;
    }
    else {
        // Brak tekstury (czysty kolor wierzcho³ków)
        baseColor = vColor.rgb;
        alpha = vColor.a;
    }

    // Ambient - delikatne œwiat³o sta³e sceny
    vec3 ambient = 0.55 * baseColor; 
    vec3 diffuseAccumulator = vec3(0.0);

    // Pêtla obliczaj¹ca œwiat³o od ka¿dej latarni miejskiej
    for(int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fragPos.xyz);
        float diff = max(dot(normal, lightDir), 0.0);
        
        float distance = length(lightPositions[i] - fragPos.xyz);
        float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.001 * (distance * distance));
        
        diffuseAccumulator += diff * vec3(1.0, 0.9, 0.6) * attenuation;
    }

    // Ostateczne wyznaczenie koloru piksela
    if (vColor.r > 0.9 && vColor.g > 0.8 && useTexture == 0) {
        pixelColor = vColor;
    } else {
        // Dla trawy mieszamy œwiat³o latarni z rozjaœnion¹ baz¹ warstwow¹
        pixelColor = vec4((ambient + diffuseAccumulator) * baseColor, alpha);
    }
}