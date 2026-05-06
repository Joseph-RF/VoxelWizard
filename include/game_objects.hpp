#pragma once
#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "shader.hpp"

enum class BlockType {
    Grass,
    Air
};

class Block {
public:
    Block();
    Block(BlockType block_type);
    ~Block();

    bool isActive();
    void setActive(bool active);

    BlockType getBlockType();
    void setBlockType(BlockType block_type);

    static constexpr float side_length = 1.0f;

    static std::map<BlockType, glm::vec3> block_type_colours;

private:
    bool active = true;
    BlockType type;
};

struct ChunkPos {
    int x;
    int y;
    int z;

    ChunkPos();
    ChunkPos(int x, int y, int z);

    bool operator==(const ChunkPos &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

class HashFunction {
public:
    size_t operator()(const ChunkPos &chunk_pos) const {
        return ((std::hash<int>()(chunk_pos.x)
                ^ (std::hash<int>()(chunk_pos.y) << 1)) >> 1)
                ^ (std::hash<int>()(chunk_pos.z) << 1);
    }
};

struct Mesh {
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    // Won't be using copy constructor or copy assignment
    Mesh(const Mesh& mesh) = delete;
    Mesh& operator=(const Mesh& mesh) = delete;

    Mesh(Mesh&& mesh) noexcept;
    Mesh& operator=(Mesh&& mesh) noexcept;

    ~Mesh();

    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int vao = 0;
};

class Chunk {
public:
    Chunk();
    Chunk(ChunkPos chunk_pos);

    // Won't be using copy constructor or copy assignment (similar to mesh)
    Chunk(const Chunk& chunk) = delete;
    Chunk& operator=(const Chunk& chunk) = delete;

    Chunk(Chunk&& chunk) noexcept;
    Chunk& operator=(Chunk&& chunk) noexcept;

    ~Chunk();

    void update(float dt);
    void draw(Shader shader);

    void generateMesh();
    void destroyMesh();

    std::vector<std::vector<std::vector<Block>>> blocks;

    static constexpr int CHUNK_SIZE = 16;

    ChunkPos chunk_pos;
    bool mesh_generated = false;
    // bool modified; TODO: Use this when block edition is added

private:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::unique_ptr<Mesh> mesh;

    bool dirtied = true;

    void generateCube(int x, int y, int z);
    unsigned int addVertex(glm::vec3 vertex, glm::vec3 colour);
    void addTriangleIndices(unsigned int v0, unsigned int v1, unsigned int v2);
};