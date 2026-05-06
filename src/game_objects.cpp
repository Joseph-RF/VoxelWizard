#include "game_objects.hpp"

#include <glad/glad.h>

#include <array>

std::map<BlockType, glm::vec3> Block::block_type_colours {
    {BlockType::Air, {1.0, 1.0, 1.0}}, // Air (shouldn't be drawn in the first place
    {BlockType::Grass, {0.15, 0.6, 0.0}}
};

Block::Block() {
    active = false;
    type = BlockType::Air;
}

Block::Block(BlockType block_type) {
    this->type = block_type;
}

Block::~Block() {
}

bool Block::isActive() {
    return active;
}

void Block::setActive(bool active) {
    this->active = active;
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

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Mesh::Mesh(Mesh &&mesh) noexcept {

    // Moving buffer data from mesh to this.
    this->vbo = mesh.vbo;
    this->ebo = mesh.ebo;
    this->vao = mesh.vao;

    mesh.vbo = 0;
    mesh.ebo = 0;
    mesh.vao = 0;
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

    mesh.vbo = 0;
    mesh.ebo = 0;
    mesh.vao = 0;

    return *this;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}

Chunk::Chunk() {
    std::cout<<"Default constructor used"<<std::endl;
    blocks = std::vector<std::vector<std::vector<Block>>>(
        CHUNK_SIZE, std::vector<std::vector<Block>>(CHUNK_SIZE, std::vector<Block>(CHUNK_SIZE))
    );
    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int j = 0; j < CHUNK_SIZE; j++) {
            for (unsigned int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i][j][k] = Block(BlockType::Grass);
            }
        }
    }
}

Chunk::Chunk(ChunkPos chunk_pos) : chunk_pos(chunk_pos) {
    blocks = std::vector<std::vector<std::vector<Block>>>(
        CHUNK_SIZE, std::vector<std::vector<Block>>(CHUNK_SIZE, std::vector<Block>(CHUNK_SIZE))
    );
    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int j = 0; j < CHUNK_SIZE; j++) {
            for (unsigned int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i][j][k] = Block(BlockType::Grass);
            }
        }
    }
}

Chunk::Chunk(Chunk &&chunk) noexcept {

    this->blocks = std::move(chunk.blocks);
    this->chunk_pos = chunk.chunk_pos;
    this->mesh_generated = chunk.mesh_generated;

    this->vertices = std::move(chunk.vertices);
    this->indices = std::move(chunk.indices);
    this->dirtied = chunk.dirtied;

    // Moving mesh from chunk to this.
    this->mesh = std::move(chunk.mesh);

    chunk.mesh = nullptr; // Is this necessary?
}

Chunk& Chunk::operator=(Chunk &&chunk) noexcept {
    if (this == &chunk) {
        return *this;
    }

    this->blocks = std::move(chunk.blocks);
    this->chunk_pos = chunk.chunk_pos;
    this->mesh_generated = chunk.mesh_generated;

    this->vertices = std::move(chunk.vertices);
    this->indices = std::move(chunk.indices);
    this->dirtied = chunk.dirtied;

    // By moving the data from chunk.mesh to this->mesh, data being held by this->mesh is released and destroyed?
    // Same for the data in chunk.mesh once it is passed to this->mesh
    this->mesh = std::move(chunk.mesh);
    return *this;
}

Chunk::~Chunk() {

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
    model = glm::translate(model, static_cast<float>(Chunk::CHUNK_SIZE) * glm::vec3(chunk_pos.x, chunk_pos.y, chunk_pos.z));
    shader.setMat("model", model);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Chunk::generateMesh() {
    if (!dirtied) {
        return;
    }
    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int j = 0; j < CHUNK_SIZE; j++) {
            for (unsigned int k = 0; k < CHUNK_SIZE; k++) {
                if (!blocks[i][j][k].isActive()) {continue;}

                generateCube(i, j, k);
            }
        }
    }

    mesh = std::make_unique<Mesh>(vertices, indices);

    mesh_generated = true;
    dirtied = false;
}

void Chunk::destroyMesh() {
    mesh = nullptr;
    std::vector<float>().swap(vertices);
    std::vector<unsigned int>().swap(indices);
    mesh_generated = false;
    dirtied = true;
}

void Chunk::generateCube(int x, int y, int z) {

    // Vertices
    glm::vec3 p0 = glm::vec3(x, y + Block::side_length, z);
    glm::vec3 p1 = glm::vec3(x, y, z);
    glm::vec3 p2 = glm::vec3(x + Block::side_length, y + Block::side_length, z);
    glm::vec3 p3 = glm::vec3(x + Block::side_length, y, z);
    glm::vec3 p4 = glm::vec3(x, y + Block::side_length, z + Block::side_length);
    glm::vec3 p5 = glm::vec3(x, y, z + Block::side_length);
    glm::vec3 p6 = glm::vec3(x + Block::side_length, y + Block::side_length, z + Block::side_length);
    glm::vec3 p7 = glm::vec3(x + Block::side_length, y, z + Block::side_length);

    // Indices
    unsigned int v0, v1, v2, v3, v4, v5, v6, v7;
    glm::vec3 colour = Block::block_type_colours[blocks[x][y][z].getBlockType()];

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

unsigned int Chunk::addVertex(glm::vec3 vertex, glm::vec3 colour) {
    vertices.push_back(vertex.x);
    vertices.push_back(vertex.y);
    vertices.push_back(vertex.z);

    // NOTE: Wasteful. Only need one colour per cube. 7 unnecessary duplicates.

    vertices.push_back(colour.x);
    vertices.push_back(colour.y);
    vertices.push_back(colour.z);

    return (vertices.size() / 6) - 1;
}

void Chunk::addTriangleIndices(unsigned int v0, unsigned int v1, unsigned int v2) {
    indices.push_back(v0);
    indices.push_back(v1);
    indices.push_back(v2);
}
