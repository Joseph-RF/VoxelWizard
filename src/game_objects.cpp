#include "game_objects.hpp"

#include <glad/glad.h>

#include <array>
#include <stddef.h>

std::map<BlockType, glm::vec3> Block::block_type_colours {
    {BlockType::Air, {1.0, 1.0, 1.0}}, // Air (shouldn't be drawn in the first place
    {BlockType::Grass, {0.15, 0.6, 0.0}}
};

Block::Block() {
    type = BlockType::Air;
}

Block::Block(BlockType block_type) {
    this->type = block_type;
}

Block::~Block() {
}

BlockType Block::getBlockType() {
    return type;
}

void Block::setBlockType(BlockType block_type) {
    this->type = block_type;
}

ChunkPos::ChunkPos() {
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

ChunkPos::ChunkPos(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Mesh::Mesh(const std::vector<VertexData>& vertices, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Will use AttribIPointer to keep the data in GLSL as an int and not cast to float
    glVertexAttribIPointer(0, 1, GL_SHORT, sizeof(VertexData), (void*)(offsetof(VertexData, packed_position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(offsetof(VertexData, colour_x)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    indices_count = indices.size();
}

Mesh::Mesh(Mesh &&mesh) noexcept {

    // Moving buffer data from mesh to this.
    this->vbo = mesh.vbo;
    this->ebo = mesh.ebo;
    this->vao = mesh.vao;
    
    this->indices_count = mesh.indices_count;

    mesh.vbo = 0;
    mesh.ebo = 0;
    mesh.vao = 0;

    mesh.indices_count = 0;
}

Mesh& Mesh::operator=(Mesh &&mesh) noexcept {
    if (this == &mesh) {
        return *this;
    }

    // Have already been initialised so need delete existing buffers and vertex arrays (assuming they exist)
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    this->vbo = mesh.vbo;
    this->ebo = mesh.ebo;
    this->vao = mesh.vao;

    this->indices_count = mesh.indices_count;

    mesh.vbo = 0;
    mesh.ebo = 0;
    mesh.vao = 0;

    mesh.indices_count = 0;

    return *this;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}

Chunk::Chunk() {
    std::cout<<"Default constructor used"<<std::endl;
    blocks = std::vector<Block>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

    std::fill(block_occupancy_x.begin(), block_occupancy_x.end(), 0);
    std::fill(block_occupancy_y.begin(), block_occupancy_y.end(), 0);
    std::fill(block_occupancy_z.begin(), block_occupancy_z.end(), 0);

    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int j = 0; j < CHUNK_SIZE; j++) {
            for (unsigned int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ] = Block(BlockType::Grass);

                int bit = blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType() == BlockType::Air ? 0 : 1;

                // Storing the occupancy of blocks in the three arrays for the three directions
                // Adding 1 bit on either side for padding
                block_occupancy_x[j + k * CHUNK_SIZE] |= (bit << (CHUNK_SIZE - i)); // Bits on the left represent blocks on the left
                block_occupancy_y[i + k * CHUNK_SIZE] |= (bit << (CHUNK_SIZE - j)); // Bits on the left represent blocks on the bottom
                block_occupancy_z[i + j * CHUNK_SIZE] |= (bit << (k + 1)); // Bits on the left represent blocks at the front
            }
        }
    }
}

Chunk::Chunk(ChunkPos chunk_pos) : chunk_pos(chunk_pos) {
    blocks = std::vector<Block>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

    std::fill(block_occupancy_x.begin(), block_occupancy_x.end(), 0);
    std::fill(block_occupancy_y.begin(), block_occupancy_y.end(), 0);
    std::fill(block_occupancy_z.begin(), block_occupancy_z.end(), 0);

    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int j = 0; j < CHUNK_SIZE; j++) {
            for (unsigned int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ] = Block(BlockType::Grass);

                int bit = blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType() == BlockType::Air ? 0 : 1;

                // Storing the occupancy of blocks in the three arrays for the three directions
                // Adding 1 bit on either side for padding
                // Will fill left to right
                block_occupancy_x[j + k * CHUNK_SIZE] |= (bit << (CHUNK_SIZE - i)); // Bits on the left represent blocks on the left
                block_occupancy_y[i + k * CHUNK_SIZE] |= (bit << (CHUNK_SIZE - j)); // Bits on the left represent blocks on the bottom
                block_occupancy_z[i + j * CHUNK_SIZE] |= (bit << (k + 1)); // Bits on the left represent blocks at the front
            }
        }
    }
}

Chunk::Chunk(Chunk &&chunk) noexcept {

    std::cout << "Move constructor called" << std::endl;

    this->blocks = std::move(chunk.blocks);
    this->block_occupancy_x = std::move(chunk.block_occupancy_x);
    this->block_occupancy_y = std::move(chunk.block_occupancy_y);
    this->block_occupancy_z = std::move(chunk.block_occupancy_z);
    this->chunk_pos = chunk.chunk_pos;
    this->vertices_generated = chunk.vertices_generated;
    this->mesh_generated = chunk.mesh_generated;

    this->vertices = std::move(chunk.vertices);
    this->vertices_back_buffer = std::move(chunk.vertices_back_buffer);
    this->indices = std::move(chunk.indices);
    this->indices_back_buffer = std::move(chunk.indices_back_buffer);
    this->back_buffers_ready = chunk.back_buffers_ready;

    this->dirtied = chunk.dirtied;

    // Moving mesh from chunk to this.
    this->mesh = std::move(chunk.mesh);

    chunk.mesh = nullptr; // Is this necessary?
}

Chunk& Chunk::operator=(Chunk &&chunk) noexcept {

    std::cout << "Move assignment operator called" << std::endl;

    if (this == &chunk) {
        return *this;
    }

    this->blocks = std::move(chunk.blocks);
    this->block_occupancy_x = std::move(chunk.block_occupancy_x);
    this->block_occupancy_y = std::move(chunk.block_occupancy_y);
    this->block_occupancy_z = std::move(chunk.block_occupancy_z);
    this->chunk_pos = chunk.chunk_pos;
    this->vertices_generated = chunk.vertices_generated;
    this->mesh_generated = chunk.mesh_generated;

    this->vertices = std::move(chunk.vertices);
    this->vertices_back_buffer = std::move(chunk.vertices_back_buffer);
    this->indices = std::move(chunk.indices);
    this->indices_back_buffer = std::move(chunk.indices_back_buffer);
    this->indices_back_buffer = chunk.indices_back_buffer;

    this->dirtied = chunk.dirtied;

    // By moving the data from chunk.mesh to this->mesh, data being held by this->mesh is released and destroyed?
    // Same for the data in chunk.mesh once it is passed to this->mesh
    this->mesh = std::move(chunk.mesh);
    return *this;
}

void Chunk::draw(Shader shader) {

    if (mesh == nullptr) {
        std::cout << "Chunk at chunk position: " << chunk_pos.x << " " << chunk_pos.y << " " << chunk_pos.z <<
                " is trying to draw but its mesh hasn't been generated yet" << std::endl;
        return;
    }

    glBindVertexArray(mesh->vao);
    shader.use();

    glm::mat4 model(1.0);
    model = glm::translate(model, static_cast<float>(Chunk::CHUNK_SIZE * Block::side_length) 
                           * glm::vec3(chunk_pos.x, chunk_pos.y, chunk_pos.z)
    );
    shader.setMat("model", model);

    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Chunk::generateVertices() {
    if (!dirtied) {
        return;
    }

    // Empty buffers just in case
    std::vector<VertexData>().swap(vertices_back_buffer);
    std::vector<unsigned int>().swap(indices_back_buffer);

    // Iterate through the x direction occupancy array first
    for (int k = 0; k < CHUNK_SIZE; ++k) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            // Left shift to see if right face needs to be drawn

            int32_t row = block_occupancy_x[j + k * CHUNK_SIZE];
            int32_t right_faces = row & ~(row << 1);
            int32_t left_faces = row & ~(row >> 1);

            glm::i32vec3 p0, p1, p2, p3;
            
            unsigned int v0, v1, v2, v3;

            glm::vec3 colour;

            for (int i = 0; i < CHUNK_SIZE; i++) {
                if (((right_faces >> (CHUNK_SIZE - i)) & 1) == 1) {
                    p0 = glm::vec3(i + 1, j + 1, k + 1);
                    p1 = glm::vec3(i + 1, j, k + 1);
                    p2 = glm::vec3(i + 1, j + 1, k);
                    p3 = glm::vec3(i + 1, j, k);

                    colour = Block::block_type_colours[blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType()];

                    v0 = addVertex(p0, colour);
                    v1 = addVertex(p1, colour);
                    v2 = addVertex(p2, colour);
                    v3 = addVertex(p3, colour);

                    addTriangleIndices(v0, v1, v2);
                    addTriangleIndices(v1, v3, v2);
                }
                if (((left_faces >> (CHUNK_SIZE - i)) & 1) == 1) {
                    p0 = glm::vec3(i, j + 1, k);
                    p1 = glm::vec3(i, j, k);
                    p2 = glm::vec3(i, j + 1, k + 1);
                    p3 = glm::vec3(i, j, k + 1);

                    colour = Block::block_type_colours[blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType()];

                    v0 = addVertex(p0, colour);
                    v1 = addVertex(p1, colour);
                    v2 = addVertex(p2, colour);
                    v3 = addVertex(p3, colour);

                    addTriangleIndices(v0, v1, v2);
                    addTriangleIndices(v1, v3, v2);
                }
            }
        }
    }

    // Iterate through the y direction occupancy array
    for (int k = 0; k < CHUNK_SIZE; ++k) {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            // Left shift to see if top face needs to be drawn
            // Right shift to see if the bottom face needs to be drawn

            int32_t row = block_occupancy_y[i + k * CHUNK_SIZE];
            int32_t top_faces = row & ~(row << 1);
            int32_t bottom_faces = row & ~(row >> 1);

            glm::i32vec3 p0, p1, p2, p3;

            unsigned int v0, v1, v2, v3;

            glm::vec3 colour;

            for (int j = 0; j < CHUNK_SIZE; j++) {
                if (((top_faces >> (CHUNK_SIZE - j)) & 1) == 1) {
                    p0 = glm::vec3(i, j + 1, k);
                    p1 = glm::vec3(i, j + 1, k + 1);
                    p2 = glm::vec3(i + 1, j + 1, k);
                    p3 = glm::vec3(i + 1, j + 1, k + 1);

                    colour = Block::block_type_colours[blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType()];

                    v0 = addVertex(p0, colour);
                    v1 = addVertex(p1, colour);
                    v2 = addVertex(p2, colour);
                    v3 = addVertex(p3, colour);

                    addTriangleIndices(v0, v1, v2);
                    addTriangleIndices(v1, v3, v2);
                }
                if (((bottom_faces >> (CHUNK_SIZE - j)) & 1) == 1) {
                    p0 = glm::vec3(i, j, k + 1);
                    p1 = glm::vec3(i, j, k);
                    p2 = glm::vec3(i + 1, j, k + 1);
                    p3 = glm::vec3(i + 1, j, k);

                    colour = Block::block_type_colours[blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType()];

                    v0 = addVertex(p0, colour);
                    v1 = addVertex(p1, colour);
                    v2 = addVertex(p2, colour);
                    v3 = addVertex(p3, colour);

                    addTriangleIndices(v0, v1, v2);
                    addTriangleIndices(v1, v3, v2);
                }
            }
        }
    }

    // Iterate through the z direction occupancy array
    for (int j = 0; j < CHUNK_SIZE; ++j) {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            // Left shift to see if back face needs to be drawn
            // Right shift to see if the front face needs to be drawn

            int32_t row = block_occupancy_z[i + j * CHUNK_SIZE];
            int32_t back_faces = row & ~(row << 1);
            int32_t front_faces = row & ~(row >> 1);

            glm::i32vec3 p0, p1, p2, p3;

            unsigned int v0, v1, v2, v3;

            glm::vec3 colour;

            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (((back_faces >> (k + 1)) & 1) == 1) {
                    p0 = glm::vec3(i + 1, j + 1, k);
                    p1 = glm::vec3(i + 1, j, k);
                    p2 = glm::vec3(i, j + 1, k);
                    p3 = glm::vec3(i, j, k);

                    colour = Block::block_type_colours[blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType()];

                    v0 = addVertex(p0, colour);
                    v1 = addVertex(p1, colour);
                    v2 = addVertex(p2, colour);
                    v3 = addVertex(p3, colour);

                    addTriangleIndices(v0, v1, v2);
                    addTriangleIndices(v1, v3, v2);
                }
                if (((front_faces >> (k + 1)) & 1) == 1) {
                    p0 = glm::vec3(i, j + 1, k + 1);
                    p1 = glm::vec3(i, j, k + 1);
                    p2 = glm::vec3(i + 1, j + 1, k + 1);
                    p3 = glm::vec3(i + 1, j, k + 1);

                    colour = Block::block_type_colours[blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ].getBlockType()];

                    v0 = addVertex(p0, colour);
                    v1 = addVertex(p1, colour);
                    v2 = addVertex(p2, colour);
                    v3 = addVertex(p3, colour);

                    addTriangleIndices(v0, v1, v2);
                    addTriangleIndices(v1, v3, v2);
                }
            }
        }
    }

    vertices_generated = true;
    dirtied = false;
}

void Chunk::generateMesh() {
    if (mesh_generated) {return;}

    mesh = std::make_unique<Mesh>(vertices, indices);

    mesh_generated = true;
}

void Chunk::swapVertexBuffers() {
    vertices_back_buffer.swap(vertices);
    indices_back_buffer.swap(indices);
}

void Chunk::destroyMesh() {
    mesh = nullptr;
    std::vector<VertexData>().swap(vertices);
    std::vector<VertexData>().swap(vertices_back_buffer);
    std::vector<unsigned int>().swap(indices);
    std::vector<unsigned int>().swap(indices_back_buffer);
    mesh_generated = false;
    vertices_generated = false;
    dirtied = true;
}

void Chunk::generateCube(int x, int y, int z) {

    // Vertices
    glm::i32vec3 p0{ x, y + 1, z };
    glm::i32vec3 p1{ x, y, z };
    glm::i32vec3 p2{ x + 1, y + 1, z };
    glm::i32vec3 p3{ x + 1, y, z };
    glm::i32vec3 p4{ x, y + 1, z + 1 };
    glm::i32vec3 p5{ x, y, z + 1 };
    glm::i32vec3 p6{ x + 1, y + 1, z + 1 };
    glm::i32vec3 p7{ x + 1, y, z + 1 };

    // Indices
    unsigned int v0, v1, v2, v3, v4, v5, v6, v7;
    glm::vec3 colour = Block::block_type_colours[blocks[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQ].getBlockType()];

    v0 = addVertex(p0, colour);
    v1 = addVertex(p1, colour);
    v2 = addVertex(p2, colour);
    v3 = addVertex(p3, colour);
    v4 = addVertex(p4, colour);
    v5 = addVertex(p5, colour);
    v6 = addVertex(p6, colour);
    v7 = addVertex(p7, colour);

    addTriangleIndices(v0, v1, v2); // Front face triangle
    addTriangleIndices(v1, v3, v2); // Front face triangle
    addTriangleIndices(v1, v5, v3); // Bottom face triangle
    addTriangleIndices(v5, v7, v3); // Bottom face triangle
    addTriangleIndices(v4, v5, v0); // Left face triangle
    addTriangleIndices(v5, v1, v0); // Left face triangle
    addTriangleIndices(v6, v7, v4); // Back face triangle
    addTriangleIndices(v7, v5, v4); // Back face triangle
    addTriangleIndices(v4, v0, v6); // Top face triangle
    addTriangleIndices(v0, v2, v6); // Top face triangle
    addTriangleIndices(v2, v3, v6); // Right face triangle
    addTriangleIndices(v3, v7, v6); // Right face triangle
}

unsigned int Chunk::addVertex(glm::i32vec3 vertex, glm::vec3 colour) {
    // TODO: NOTE, must change this if chunks change from 16x16x16
    // Packing positional information for three axes into a single 16 bit int
    // x axis will be five right most bits (5 needed for 17 different positions)
    short int vertex_3D = 0;
    vertex_3D |= vertex.x;

    vertex_3D |= (vertex.y << 5);

    vertex_3D |= (vertex.z << 10);

    // NOTE: Wasteful. Only need one colour per cube. 7 unnecessary duplicates.

    VertexData vertex_data{ vertex_3D, colour.x, colour.y, colour.z };

    vertices_back_buffer.push_back(vertex_data);

    return vertices_back_buffer.size() - 1;
}

void Chunk::addTriangleIndices(unsigned int v0, unsigned int v1, unsigned int v2) {
    indices_back_buffer.push_back(v0);
    indices_back_buffer.push_back(v1);
    indices_back_buffer.push_back(v2);
}
