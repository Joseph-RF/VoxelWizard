#version 330 core

layout (location = 0) in int aPos;
layout (location = 1) in vec3 aColour;

uniform float side_length;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 fragment_pos;
out vec3 colour;

const vec3 normals[6] = vec3[6](
    vec3(-1.0,  0.0,  0.0), // -X
    vec3( 1.0,  0.0,  0.0), // +X
    vec3( 0.0, -1.0,  0.0), // -Y
    vec3( 0.0,  1.0,  0.0), // +Y
    vec3( 0.0,  0.0, -1.0), // -Z
    vec3( 0.0,  0.0,  1.0)  // +Z
);

void main() {

    colour = aColour;

    // NOTE: Will need to change this if the size of the chunks ever change from 16x16x16
    vec3 position = side_length * vec3((aPos & 31),
                                      ((aPos & (31 << 5)) >> 5), 
                                      ((aPos & (31 << 10)) >> 10)
    );

    int index = (aPos & (7 << 15)) >> 15;
    normal = normals[index];
    //normal = mat3(transpose(inverse(model))) * normals[index]; // TODO: Check if it is necessary to do this

    fragment_pos = vec3(model * vec4(position, 1.0)); // Need the world position of the fragment

    gl_Position = projection * view * model * vec4(position, 1.0); // Cast aPos to a vec4
};