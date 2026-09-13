#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;

out vec3 normal;
out vec3 fragment_pos;
out vec3 colour;

layout (std140) uniform Matrices {
    mat4 projection;         // Starts at 0,   ends at 64
    mat4 view;               // Starts at 64,  ends at 128
};

void main() {

    vec3 position = aPos;
    
    normal = mat3(transpose(inverse(model))) * aNormal; // TODO: Check if it is necessary to do this
    colour = aColour;
    fragment_pos = vec3(model * vec4(position, 1.0)); // Need the world position of the fragment

    gl_Position = projection * view * model * vec4(position, 1.0); // Cast position to a vec4
};