#version 330 core

layout (location = 0) in int aPos;
layout (location = 1) in vec3 aColour;

uniform float side_length;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 colour;

void main() {
    colour = vec4(aColour, 1.0);

    // NOTE: Will need to change this if the size of the chunks ever change from 16x16x16
    vec3 position = side_length * vec3((aPos & 31),
                                      ((aPos & (31 << 5)) >> 5), 
                                      ((aPos & (31 << 10)) >> 10)
    );

    //colour = vec4(position.x / (side_length * 16), position.y / (side_length * 16), position.z / (side_length * 16), 1.0);

    gl_Position = projection * view * model * vec4(position, 1.0); // Cast aPos to a vec4
};