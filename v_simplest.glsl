#version 330 core

in vec4 vertex;
in vec4 normal;

uniform mat4 P;
uniform mat4 V; 
uniform mat4 M; 

out vec3 vNormal;
out vec3 fragPos; 

void main() {
    vNormal = normalize(mat3(M) * normal.xyz);
    
    vec4 worldPos = M * vertex;
    fragPos = worldPos.xyz; 
    
    gl_Position = P * V * worldPos;
}