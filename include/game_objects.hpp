#pragma once
#include <vector>
#include <map>
#include <memory>
#include <array>
#include <string>

#include <glm/glm.hpp>
#include "../third_party/fastnoiselite/FastNoiseLite.h"

#include "shader.hpp"

enum class BlockType {
    Grass,
    Stone,
    Snow,
    Sand,
    Water,
    Air
};

BlockType heightToBlockType(int height);

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
    static std::map<BlockType, int8_t> block_type_to_int8;
    static std::map<int8_t, glm::vec3> int8_to_colour;

    static constexpr int32_t LEFT_FACE = 0;
    static constexpr int32_t RIGHT_FACE = 1;
    static constexpr int32_t DOWN_FACE = 2;
    static constexpr int32_t UP_FACE = 3;
    static constexpr int32_t BEHIND_FACE = 4;
    static constexpr int32_t IN_FRONT_FACE = 5;

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
    int32_t packed_vertex_data;
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
    unsigned int instance_array_vbo = 0;
    unsigned int vao = 0;
};

class Chunk {
public:
    Chunk();
    Chunk(ChunkPos chunk_pos, const std::vector<int>& height_map);

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
    
    void destroyMesh();

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_SIZE_SQ = CHUNK_SIZE * CHUNK_SIZE;

    std::vector<int8_t> blocks;
    std::vector<int32_t> block_occupancy_x;
    std::vector<int32_t> block_occupancy_y;
    std::vector<int32_t> block_occupancy_z;

    ChunkPos chunk_pos;
    bool vertices_generated = false;
    bool mesh_generated = false;
    // bool modified; TODO: Use this when block edition is added
    bool do_not_draw = false;
    bool empty = true;

private:
    std::vector<VertexData> vertices;
    std::vector<unsigned int> indices;

    std::unique_ptr<Mesh> mesh;

    bool dirtied = true;
    int face_count = 0;

    void addQuad(glm::i32vec3 position, int8_t block_id, int32_t face);
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
    void generateMeshes();
    void refreshMeshes();
    void destroyMeshes();

    static constexpr int COLUMN_HEIGHT = 16;
    static constexpr int SEA_LEVEL = 16;
    static constexpr int MOUNTAIN_PEAK = 200;

    ColumnPos column_pos;
    std::vector<Chunk> chunks; // 0 will be the chunk at the bottom, 15 will be the chunk at the top

    // Perlin noise generator
    static void configureNoiseGenerator();
    static FastNoiseLite noise;
};