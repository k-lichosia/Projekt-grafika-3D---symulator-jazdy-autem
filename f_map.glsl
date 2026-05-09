#version 330
out vec4 pixelColor;
in vec4 i_color;

void main() {
    pixelColor = i_color;
}