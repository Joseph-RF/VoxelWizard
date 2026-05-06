#version 330 core

in vec4 colour;

out vec4 FragColor; // Fragment shader only requires a single output variable

void main() {
    FragColor = vec4(colour.x, colour.y, colour.z, 1.0f); // Output must be vec4
};