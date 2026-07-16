#pragma once
#include <vector>
#include <map>
#include <memory>
#include <array>

#include <glm/glm.hpp>

#include "shader.hpp"

enum class BlockType {
    Grass,
    Air
};

enum class ChunkNeighbour {
    LEFT, // -X
    RIGHT, // +X
    BELOW, // -Y
    ABOVE, // +Y
    BEHIND, // -Z
    IN_FRONT // +Z
};

ChunkNeighbour reverseChunkNeighbour(ChunkNeighbour chunk_neighbour);

class Block {
public:
    Block();
    Block(BlockType block_type);
    ~Block();

    BlockType getBlockType();
    void setBlockType(BlockType block_type);

    static constexpr float side_length = 1.0f;

    static std::map<BlockType, glm::vec3> block_type_colours;

private:
    BlockType type;
};

struct ColumnPos {
    int x;
    int z;

    ColumnPos();
    ColumnPos(int x, int z);

    bool operator==(const ColumnPos& other) const {
        return x == other.x && z == other.z;
    }
};

struct ChunkPos {
    int x;
    int y;
    int z;

    ChunkPos();
    ChunkPos(int x, int y, int z);

    bool operator==(const ChunkPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    ColumnPos getColumnPos() {
        return ColumnPos(x, z);
    }
};

class ChunkPosHashFunc {
public:
    std::size_t operator()(const ChunkPos& chunk_pos) const noexcept {
        std::size_t h = std::hash<int>{}(chunk_pos.x);
        h ^= std::hash<int>{}(chunk_pos.y) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(chunk_pos.z) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        return h;
    }
};

class ColumnPosHashFunc {
public:
    std::size_t operator()(const ColumnPos& column_pos) const noexcept {
        std::size_t h1 = std::hash<int>{}(column_pos.x);
        std::size_t h2 = std::hash<int>{}(column_pos.z);
        // boost-style combine
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

struct VertexData {
    short int packed_position;
    float colour_x;
    float colour_y;
    float colour_z;
};

struct Mesh {
    Mesh(const std::vector<VertexData>& vertices, const std::vector<unsigned int>& indices);

    // Won't be using copy constructor or copy assignment
    Mesh(const Mesh& mesh) = delete;
    Mesh& operator=(const Mesh& mesh) = delete;

    Mesh(Mesh&& mesh) noexcept;
    Mesh& operator=(Mesh&& mesh) noexcept;

    ~Mesh();

    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int vao = 0;

    unsigned int indices_count = 0;
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

    ~Chunk() = default;

    void update(float dt);
    void draw(Shader& shader);

    void setDirty(bool dirty);

    void generateVertices(); // Vertex data created here
    void generateMesh(); // Buffers sent to GPU here (OpenGL call)
    void refreshMesh();
    
    void padOccupancy(const Chunk& source, const ChunkNeighbour neighbour_position);

    void swapVertexBuffers();
    
    void destroyMesh();

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_SIZE_SQ = CHUNK_SIZE * CHUNK_SIZE;

    std::vector<Block> blocks;
    std::array<int32_t, CHUNK_SIZE_SQ> block_occupancy_x;
    std::array<int32_t, CHUNK_SIZE_SQ> block_occupancy_y;
    std::array<int32_t, CHUNK_SIZE_SQ> block_occupancy_z;

    ChunkPos chunk_pos;
    bool vertices_generated = false;
    bool mesh_generated = false;
    // bool modified; TODO: Use this when block edition is added

private:
    std::vector<VertexData> vertices;
    std::vector<VertexData> vertices_back_buffer;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> indices_back_buffer;
    bool back_buffers_ready = false;

    std::unique_ptr<Mesh> mesh;

    bool dirtied = true;

    unsigned int addVertex(glm::i32vec3 position, glm::vec3 colour);
    void addTriangleIndices(unsigned int v0, unsigned int v1, unsigned int v2);
};

struct Column {
    Column(ColumnPos column_pos);

    // Won't be using copy constructor or copy assignment (similar to mesh)
    Column(const Column& column) = delete;
    Column& operator=(const Column& column) = delete;

    Column(Column&& column) noexcept;
    Column& operator=(Column&& column) noexcept;

    ~Column() = default;

    Chunk& operator[](int index);
    const Chunk& operator[](int index) const;

    void draw(Shader& shader);

    void generateVertices();
    void swapVertexBuffers();
    void generateMeshes();
    void refreshMeshes();
    void destroyMeshes();

    static constexpr int COLUMN_HEIGHT = 16;

    ColumnPos column_pos;
    std::vector<Chunk> chunks; // 0 will be the chunk at the bottom, 15 will be the chunk at the top
};