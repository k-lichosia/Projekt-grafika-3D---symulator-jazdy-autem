#version 330
layout(location = 0) in vec4 vertex; // Pozycja z roadVertices
layout(location = 1) in vec4 color;  // Kolor z roadColors

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec4 i_color;

void main() {
    i_color = color;
    gl_Position = P * V * M * vertex;
}