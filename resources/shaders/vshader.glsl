#version 330 core

layout (location = 0) in int aPos;

uniform float side_length;

uniform mat4 model;

out vec3 normal;
out vec3 fragment_pos;
out vec3 colour;

layout (std140) uniform buffer_objects {
    mat4 projection;         // Starts at 0,   ends at 64
    mat4 view;               // Starts at 64,  ends at 128
    vec3 normals[6];         // Starts at 128, ends at 224
    vec3 block_colours[6];   // Starts at 224, ends at 320
};

void main() {

    // NOTE: Will need to change this if the size of the chunks ever change from 16x16x16
    vec3 position = side_length * vec3((aPos & 31),
                                      ((aPos & (31 << 5)) >> 5), 
                                      ((aPos & (31 << 10)) >> 10)
    );

    int normal_index = (aPos & (7 << 15)) >> 15;
    normal = normals[normal_index];
    //normal = mat3(transpose(inverse(model))) * normals[index]; // TODO: Check if it is necessary to do this

    int block_index = (aPos & (255 << 18)) >> 18;
    colour = block_colours[block_index];

    fragment_pos = vec3(model * vec4(position, 1.0)); // Need the world position of the fragment

    gl_Position = projection * view * model * vec4(position, 1.0); // Cast aPos to a vec4
};