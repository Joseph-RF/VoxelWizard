#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int packed_vertex_data;

uniform float side_length;

uniform mat4 model;

out vec3 normal;
out vec3 fragment_pos;
out vec3 colour;

layout (std140) uniform Matrices {
    mat4 projection;         // Starts at 0,   ends at 64
    mat4 view;               // Starts at 64,  ends at 128
};

layout (std140) uniform BlockData {
    vec3 normals[6];         // Starts at 0, ends at 96
    vec3 block_colours[6];   // Starts at 96, ends at 192
};

vec3 topToXFace(vec3 pos, int face_id);

void main() {

    // NOTE: Will need to change this if the size of the chunks ever change from 16x16x16
    // Back lower left corner position of the voxel this vertex belongs to
    vec3 voxel_position = side_length * vec3((packed_vertex_data & 15),
                                      ((packed_vertex_data & (15 << 4)) >> 4), 
                                      ((packed_vertex_data & (15 << 8)) >> 8)
    );

    int normal_index = (packed_vertex_data & (7 << 12)) >> 12;
    normal = normals[normal_index];
    //normal = mat3(transpose(inverse(model))) * normals[index]; // TODO: Check if it is necessary to do this

    // topToXFace should return unit square vertices
    vec3 vertex_position = side_length * topToXFace(aPos, normal_index);

    vec3 position = vertex_position + voxel_position;

    int block_index = (packed_vertex_data & (255 << 15)) >> 15;
    colour = block_colours[block_index];

    fragment_pos = vec3(model * vec4(position, 1.0)); // Need the world position of the fragment

    gl_Position = projection * view * model * vec4(position, 1.0); // Cast position to a vec4
};

vec3 topToXFace(vec3 pos, int face_id) {

    vec3 return_pos = pos;

    if(face_id == 0) {
        // Top face to Left face
        // Rotate -90 about y
        // Rotate 90 about z
        // Move +0.5 in y, +0.5 in z
        mat3 rotation_matrix = mat3(
            vec3(0.0, 0.0, 1.0),
            vec3(-1.0, 0.0, 0.0),
            vec3(0.0, -1.0, 0.0)
        );
        return_pos = rotation_matrix * return_pos;
        return_pos = vec3(return_pos.x, return_pos.y + 0.5, return_pos.z + 0.5);

        return return_pos;
    }
    if(face_id == 1) {
        // Top face to Right face
        // Rotate 90 about y
        // Rotate -90 about z
        // Move +1.0 units in x, +0.5 units in y, +0.5 units in z
        mat3 rotation_matrix = mat3(
            vec3(0.0, 0.0, -1.0),
            vec3(1.0, 0.0, 0.0),
            vec3(0.0, -1.0, 0.0)
        );
        return_pos = rotation_matrix * return_pos;
        return_pos = vec3(return_pos.x + 1, return_pos.y + 0.5, return_pos.z + 0.5);

        return return_pos;
    }
    if(face_id == 2) {
        // Top face to Bottom face
        // Rotate 180 about x
        // Move +0.5 units in x, +0.5 units in z
        mat3 rotation_matrix = mat3(
            vec3(1.0, 0.0, 0.0),
            vec3(0.0, -1.0, 0.0),
            vec3(0.0, 0.0, -1.0)
        );
        return_pos = rotation_matrix * return_pos;
        return_pos = vec3(return_pos.x + 0.5, return_pos.y, return_pos.z + 0.5);

        return return_pos;
    }
    if(face_id == 3) {
        // Top face to Top face
        // Move +0.5 units in x, +1.0 units in y, +0.5 units in z
        return vec3(return_pos.x + 0.5, return_pos.y + 1, return_pos.z + 0.5);
    }
    if(face_id == 4) {
        // Top face to Back face
        // Rotate 180 about y
        // Rotate -90 about x
        // Move +0.5 units in x, +0.5 units in y
        mat3 rotation_matrix = mat3(
            vec3(-1.0, 0.0, 0.0),
            vec3(0.0, 0.0, -1.0),
            vec3(0.0, -1.0, 0.0)
        );
        return_pos = rotation_matrix * return_pos;
        return_pos = vec3(return_pos.x + 0.5, return_pos.y + 0.5, return_pos.z);

        return return_pos;
    }
    // Top face to Front face
    // Rotate 90 about x
    // Move +0.5 units in x, +0.5 units in y, +1.0 units in z
    mat3 rotation_matrix = mat3(
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, -1.0, 0.0)
    );
    return_pos = rotation_matrix * return_pos;
    return_pos = vec3(return_pos.x + 0.5, return_pos.y + 0.5, return_pos.z + 1);

    return return_pos;
}