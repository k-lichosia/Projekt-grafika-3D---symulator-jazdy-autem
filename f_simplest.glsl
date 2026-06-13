#version 330 core

in vec3 vNormal;
in vec3 fragPos;

uniform vec3 objectColor;
uniform vec3 lightPositions[10];
uniform int lightCount;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 ambient = 0.25 * objectColor;
    vec3 diffuseAccumulator = vec3(0.0);

    for(int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(lightPositions[i] - fragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        
        float distance = length(lightPositions[i] - fragPos);
        float attenuation = 1.0 / (1.0 + 0.05 * distance + 0.01 * (distance * distance));
        
        diffuseAccumulator += diff * vec3(0.8, 0.75, 0.55) * attenuation;
    }

    vec3 finalColor = ambient + (diffuseAccumulator * objectColor);

    FragColor = vec4(min(finalColor, vec3(1.0)), 1.0);
}