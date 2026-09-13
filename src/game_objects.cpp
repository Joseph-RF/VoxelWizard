#include "game_objects.hpp"

#include <glad/glad.h>

#include <array>
#include <stddef.h>

BlockType heightToBlockType(int height) {
    if (height <= 1) {
        return BlockType::Stone;
    }
    else if (height <= 16) {
        return BlockType::Water;
    }
    else if (height <= 19) {
        return BlockType::Sand;
    }
    else if (height <= 120) {
        return BlockType::Grass;
    }
    else if (height <= 140) {
        return BlockType::Stone;
    }
    else if (height <= 200) {
        return BlockType::Snow;
    }
    return BlockType::Air;
}

ChunkNeighbour reverseChunkNeighbour(ChunkNeighbour chunk_neighbour) {
    switch (chunk_neighbour) {
    case ChunkNeighbour::LEFT:
        return ChunkNeighbour::RIGHT;
        break;
    case ChunkNeighbour::RIGHT:
        return ChunkNeighbour::LEFT;
        break;
    case ChunkNeighbour::BELOW:
        return ChunkNeighbour::ABOVE;
        break;
    case ChunkNeighbour::ABOVE:
        return ChunkNeighbour::BELOW;
        break;
    case ChunkNeighbour::BEHIND:
        return ChunkNeighbour::IN_FRONT;
        break;
    case ChunkNeighbour::IN_FRONT:
        return ChunkNeighbour::BEHIND;
        break;
    default:
        std::cout << "How did this get here?" << std::endl;
        return ChunkNeighbour::LEFT;
    }
}

std::map<BlockType, glm::vec3> Block::block_type_colours {
    {BlockType::Air, {1.0, 1.0, 1.0}},
    {BlockType::Grass, {0.15, 0.6, 0.0}},
    {BlockType::Stone, {0.4, 0.4, 0.4}},
    {BlockType::Snow, {0.8, 0.8, 0.8}},
    {BlockType::Sand, {0.72, 0.67, 0.52}},
    {BlockType::Water, {0.1, 0.1, 0.8}},
};

std::map<BlockType, int8_t> Block::block_type_to_int8{
    {BlockType::Air, 0},
    {BlockType::Grass, 1},
    {BlockType::Stone, 2},
    {BlockType::Snow, 3},
    {BlockType::Sand, 4},
    {BlockType::Water, 5},
};

std::map<int8_t, glm::vec3> Block::int8_to_colour{
    {0, {1.0, 1.0, 1.0}},
    {1, {0.15, 0.6, 0.0}},
    {2, {0.4, 0.4, 0.4}},
    {3, {0.8, 0.8, 0.8}},
    {4, {0.72, 0.67, 0.52}},
    {5, {0.1, 0.1, 0.8}},
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

ChunkPos::ChunkPos(int x, int y, int z) : x(x), y(y), z(z) {}

ColumnPos::ColumnPos() : x(0), z(0) {}

ColumnPos::ColumnPos(int x, int z) : x(x), z(z) {}

Chunk::Chunk() {
    std::cout<<"Default constructor used"<<std::endl;

    blocks = std::vector<int8_t>(CHUNK_SIZE_SQ * CHUNK_SIZE, 0);

    block_occupancy_x = std::vector<int32_t>(CHUNK_SIZE_SQ, 0);
    block_occupancy_y = std::vector<int32_t>(CHUNK_SIZE_SQ, 0);
    block_occupancy_z = std::vector<int32_t>(CHUNK_SIZE_SQ, 0);

    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int j = 0; j < CHUNK_SIZE; j++) {
            for (unsigned int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ] = 1;

                int bit = blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ] == 0 ? 0 : 1;

                // Storing the occupancy of blocks in the three arrays for the three directions
                // Adding 1 bit on either side for padding
                block_occupancy_x[j + k * CHUNK_SIZE] |= (bit << (CHUNK_SIZE - i)); // Bits on the left represent blocks on the left
                block_occupancy_y[i + k * CHUNK_SIZE] |= (bit << (CHUNK_SIZE - j)); // Bits on the left represent blocks on the bottom
                block_occupancy_z[i + j * CHUNK_SIZE] |= (bit << (k + 1)); // Bits on the left represent blocks at the front
            }
        }
    }
}

Chunk::Chunk(ChunkPos chunk_pos, const std::vector<int>& height_map) : chunk_pos(chunk_pos) {

    generateBlockData(height_map);

    block_occupancy_x = std::vector<int32_t>(CHUNK_SIZE_SQ, 0);
    block_occupancy_y = std::vector<int32_t>(CHUNK_SIZE_SQ, 0);
    block_occupancy_z = std::vector<int32_t>(CHUNK_SIZE_SQ, 0);

    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int k = 0; k < CHUNK_SIZE; k++) {

            int adjusted_height = height_map[i + k * CHUNK_SIZE] - chunk_pos.y * CHUNK_SIZE;
            BlockType top_block_type = heightToBlockType(height_map[i + k * CHUNK_SIZE]);
            int8_t air_int8 = Block::block_type_to_int8[BlockType::Air];

            for (unsigned int j = 0; j < CHUNK_SIZE; ++j) {
                if (blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ] != air_int8) {

                    // Storing the occupancy of blocks in the three arrays for the three directions
                    // Adding 1 bit on either side for padding
                    // Will fill left to right
                    block_occupancy_x[j + k * CHUNK_SIZE] |= (1 << (CHUNK_SIZE - i)); // Bits on the left represent blocks on the left -x
                    block_occupancy_y[i + k * CHUNK_SIZE] |= (1 << (CHUNK_SIZE - j)); // Bits on the left represent blocks on the bottom -y
                    block_occupancy_z[i + j * CHUNK_SIZE] |= (1 << (k + 1)); // Bits on the left represent blocks at the front +z
                }
                // Don't need to do any bit manipulations if block is air since empty is the default
            }
        }
    }

    if (empty) {
        // Since the chunk is completely empty, no need to waste memory

        std::vector<int8_t>().swap(blocks);
        std::vector<int32_t>().swap(block_occupancy_x);
        std::vector<int32_t>().swap(block_occupancy_y);
        std::vector<int32_t>().swap(block_occupancy_z);
    }
}

Chunk::Chunk(Chunk &&chunk) noexcept {

    //std::cout << "Move constructor called" << std::endl;

    this->blocks = std::move(chunk.blocks);
    this->block_occupancy_x = std::move(chunk.block_occupancy_x);
    this->block_occupancy_y = std::move(chunk.block_occupancy_y);
    this->block_occupancy_z = std::move(chunk.block_occupancy_z);
    this->chunk_pos = chunk.chunk_pos;
    this->vertices_generated = chunk.vertices_generated;
    this->mesh_generated = chunk.mesh_generated;
    this->do_not_draw = chunk.do_not_draw;

    this->vertices = std::move(chunk.vertices);

    this->dirtied = chunk.dirtied;
    this->empty = chunk.empty;
    this->face_count = chunk.face_count;

    // Moving mesh from chunk to this.
    this->mesh = std::move(chunk.mesh);

    chunk.mesh = nullptr; // Is this necessary?
}

Chunk& Chunk::operator=(Chunk &&chunk) noexcept {

    //std::cout << "Move assignment operator called" << std::endl;

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
    this->do_not_draw = chunk.do_not_draw;

    this->vertices = std::move(chunk.vertices);

    this->dirtied = chunk.dirtied;
    this->empty = chunk.empty;
    this->face_count = chunk.face_count;

    // By moving the data from chunk.mesh to this->mesh, data being held by this->mesh is released and destroyed?
    // Same for the data in chunk.mesh once it is passed to this->mesh
    this->mesh = std::move(chunk.mesh);
    return *this;
}

void Chunk::generateBlockData(const std::vector<int>& height_map) {

    // Fill with default air blocks
    blocks = std::vector<int8_t>(CHUNK_SIZE * CHUNK_SIZE_SQ, Block::block_type_to_int8[BlockType::Air]);
    empty = true;

    for (unsigned int i = 0; i < CHUNK_SIZE; i++) {
        for (unsigned int k = 0; k < CHUNK_SIZE; k++) {

            int adjusted_height = height_map[i + k * CHUNK_SIZE] - chunk_pos.y * CHUNK_SIZE;
            BlockType top_block_type = heightToBlockType(height_map[i + k * CHUNK_SIZE]);

            for (unsigned int j = 0; j < CHUNK_SIZE; ++j) {
                if (j <= adjusted_height && adjusted_height > 0) {
                    blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ] = Block::block_type_to_int8[top_block_type];
                    empty = false;
                }
            }
        }
    }
}

void Chunk::draw(Shader& shader) {

    if (mesh == nullptr) {
        //std::cout << "Chunk at chunk position: " << chunk_pos.x << " " << chunk_pos.y << " " << chunk_pos.z <<
        //        " is trying to draw but its mesh hasn't been generated yet" << std::endl;
        return;
    }

    if (do_not_draw) { return; }

    glBindVertexArray(mesh->vao);
    shader.use();

    glm::mat4 model(1.0);
    model = glm::translate(model, static_cast<float>(Chunk::CHUNK_SIZE * Block::side_length)
                           * glm::vec3(chunk_pos.x, chunk_pos.y, chunk_pos.z)
    );
    shader.setMat("model", model);

    glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, face_count, 0);
    //glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, face_count);
    glBindVertexArray(0);
}

void Chunk::setDirty(bool dirty) {
    this->dirtied = dirty;
}

void Chunk::generateVertices() {
    if (!dirtied) {
        return;
    }

    // Ensure vectors are empty
    std::vector<VertexData>().swap(vertices);
    do_not_draw = false;

    if (empty) {
        vertices_generated = true;
        dirtied = false;
        do_not_draw = true;
        
        return;
    }

    // Iterate through the x direction occupancy array first
    for (int k = 0; k < CHUNK_SIZE; ++k) {
        for (int j = 0; j < CHUNK_SIZE; ++j) {
            // Left shift to see if right face needs to be drawn

            int32_t row = block_occupancy_x[j + k * CHUNK_SIZE];
            int32_t right_faces = row & ~(row << 1);
            int32_t left_faces = row & ~(row >> 1);

            int8_t block_id;

            for (int i = 0; i < CHUNK_SIZE; i++) {

                block_id = blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ];

                if (((right_faces >> (CHUNK_SIZE - i)) & 1) == 1) {

                    addQuad({i, j, k}, block_id, Block::RIGHT_FACE);
                }
                if (((left_faces >> (CHUNK_SIZE - i)) & 1) == 1) {

                    addQuad({ i, j, k }, block_id, Block::LEFT_FACE);
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

            int8_t block_id;

            for (int j = 0; j < CHUNK_SIZE; j++) {

                block_id = blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ];

                if (((top_faces >> (CHUNK_SIZE - j)) & 1) == 1) {

                    addQuad({i, j, k}, block_id, Block::UP_FACE);
                }
                if (((bottom_faces >> (CHUNK_SIZE - j)) & 1) == 1) {

                    addQuad({i, j, k}, block_id, Block::DOWN_FACE);
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

            int8_t block_id;

            for (int k = 0; k < CHUNK_SIZE; k++) {

                block_id = blocks[i + j * CHUNK_SIZE + k * CHUNK_SIZE_SQ];

                if (((back_faces >> (k + 1)) & 1) == 1) {

                    addQuad({i, j, k}, block_id, Block::BEHIND_FACE);
                }
                if (((front_faces >> (k + 1)) & 1) == 1) {

                    addQuad({i, j, k}, block_id, Block::IN_FRONT_FACE);
                }
            }
        }
    }

    vertices_generated = true;
    dirtied = false;

    if (face_count == 0) {
        do_not_draw = true;
    }
}

void Chunk::generateMesh() {
    if (mesh_generated || empty) {return;}

    mesh = std::make_unique<Mesh>(vertices);

    // Empty vertices and indices as they aren't needed
    std::vector<VertexData>().swap(vertices);

    mesh_generated = true;
}

void Chunk::refreshMesh() {
    std::unique_ptr<Mesh> temp_mesh = std::make_unique<Mesh>(vertices);
    mesh = std::move(temp_mesh);
}

void Chunk::padOccupancy(const Chunk& source, const ChunkNeighbour neighbour_position) {

    // TODO: There is for sure a better way of implementing this

    // Skip if this chunk is empty
    if (this->empty) {
        return;
    }

    switch (neighbour_position) {
    case ChunkNeighbour::LEFT:
        // If neighbour position is -x, it is on the left of this chunk.
        // Want the occupancy data of right edge of the neighbour
        for (int i = 0; i < CHUNK_SIZE_SQ; ++i) {
            if (source.empty) {
                this->block_occupancy_x[i] |= (0 & (1 << 1)) << (CHUNK_SIZE);
            }
            else {
                this->block_occupancy_x[i] |= (source.block_occupancy_x[i] & (1 << 1)) << (CHUNK_SIZE);
            }
        }
        break;
    case ChunkNeighbour::RIGHT:
        // If neigbour position is +x, it is on the right of the chunk.
        // Want the occupancy data of the left edge of the neighbour
        for (int i = 0; i < CHUNK_SIZE_SQ; ++i) {
            if (source.empty) {
                this->block_occupancy_x[i] |= (0 & (1 << CHUNK_SIZE)) >> (CHUNK_SIZE);
            }
            else {
                this->block_occupancy_x[i] |= (source.block_occupancy_x[i] & (1 << CHUNK_SIZE)) >> (CHUNK_SIZE);
            }
        }
        break;
    case ChunkNeighbour::BELOW:
        // If neighbour is -y, same as -x
        for (int i = 0; i < CHUNK_SIZE_SQ; ++i) {
            if (source.empty) {
                this->block_occupancy_y[i] |= (0 & (1 << 1)) << (CHUNK_SIZE);
            }
            else {
                this->block_occupancy_y[i] |= (source.block_occupancy_y[i] & (1 << 1)) << (CHUNK_SIZE);
            }
        }
        break;
    case ChunkNeighbour::ABOVE:
        // If neighbour is +y, same as +x
        for (int i = 0; i < CHUNK_SIZE_SQ; ++i) {
            if (source.empty) {
                this->block_occupancy_y[i] |= (0 & (1 << CHUNK_SIZE)) >> (CHUNK_SIZE);
            }
            else {
                this->block_occupancy_y[i] |= (source.block_occupancy_y[i] & (1 << CHUNK_SIZE)) >> (CHUNK_SIZE);
            }
        }
        break;
    case ChunkNeighbour::BEHIND:
        // If neighbour is -z, same as +x
        for (int i = 0; i < CHUNK_SIZE_SQ; ++i) {
            if (source.empty) {
                this->block_occupancy_z[i] |= (0 & (1 << CHUNK_SIZE)) >> (CHUNK_SIZE);
            }
            else {
                this->block_occupancy_z[i] |= (source.block_occupancy_z[i] & (1 << CHUNK_SIZE)) >> (CHUNK_SIZE);
            }
        }
        break;
    case ChunkNeighbour::IN_FRONT:
        // If neighbour is +z, same as -x
        for (int i = 0; i < CHUNK_SIZE_SQ; ++i) {
            if (source.empty) {
                this->block_occupancy_z[i] |= (0 & (1 << 1)) << (CHUNK_SIZE);
            }
            else {
                this->block_occupancy_z[i] |= (source.block_occupancy_z[i] & (1 << 1)) << (CHUNK_SIZE);
            }
        }
        break;
    default:
        std::cout << "What? How did this happen?" << std::endl;
    }
}

void Chunk::destroyMesh() {
    mesh = nullptr;
    std::vector<VertexData>().swap(vertices);
    mesh_generated = false;
    vertices_generated = false;
    dirtied = true;
}

void Chunk::addQuad(glm::i32vec3 position, int8_t block_id, int32_t face) {
    // Position corresponds to the 0, 0, 0 of the voxel a.k.a back left bottom corner
    // regardless of orientation

    // TODO: NOTE, must change this if chunks change from 16x16x16
    // Packing positional information for three axes into a single 16 bit int
    // x axis will be four right most bits (4 needed for 16 different positions)
    // 3 extra bits needed for the six different face values (15)
    // 8 bits given for the different possible block types there may be (23 total)

    int32_t packed_vertex = 0;

    // Positional data
    packed_vertex |= position.x;
    packed_vertex |= (position.y << 4);
    packed_vertex |= (position.z << 8);

    // Normal data
    packed_vertex |= (face << 12);

    // Block id data
    packed_vertex |= (block_id << 15);

    VertexData vertex_data{ packed_vertex };

    vertices.push_back(vertex_data);

    face_count++;
}

Column::Column(ColumnPos column_pos) : column_pos(column_pos) {
    // Find the height map for the column
    std::vector<int> height_map;
    generateColumnHeightMap(height_map);

    for (int i = 0; i < Column::COLUMN_HEIGHT; ++i) {
        chunks.emplace_back(ChunkPos{ column_pos.x, i, column_pos.z }, height_map);
    }
}

Column::Column(Column&& column) noexcept {
    this->column_pos = column.column_pos;
    this->chunks = std::move(column.chunks);
}

Column& Column::operator=(Column&& column) noexcept {
    if (this == &column) {
        return *this;
    }

    this->column_pos = column.column_pos;
    this->chunks = std::move(column.chunks);

    return *this;
}

Chunk& Column::operator[](int index) {
    if (index >= Column::COLUMN_HEIGHT || index < 0) {
        std::cout << "Array index out of bound, returning 0th chunk";
        return chunks[0];
    }
    return chunks[index];
}

const Chunk& Column::operator[](int index) const {
    if (index >= Column::COLUMN_HEIGHT || index < 0) {
        std::cout << "Array index out of bound, returning 0th chunk";
        return chunks[0];
    }
    return chunks[index];
}

void Column::generateColumnHeightMap(std::vector<int>& height_map) {
    height_map = std::vector<int>(Chunk::CHUNK_SIZE_SQ, 1);

    /*
    * 
    * Perlin Noise Generated Terrain
    * 
    for (int i = 0; i < Chunk::CHUNK_SIZE; ++i) {
        for (int k = 0; k < Chunk::CHUNK_SIZE; ++k) {
            int world_x_pos = i + column_pos.x * Chunk::CHUNK_SIZE;
            int world_z_pos = k + column_pos.z * Chunk::CHUNK_SIZE;

            float unit_height = (noise.GetNoise(float(world_x_pos) / 4.f, float(world_z_pos) / 4.f) + 1) * 0.5f;

            unit_height = pow(unit_height, 5);

            height_map[i + k * Chunk::CHUNK_SIZE] = unit_height * (MOUNTAIN_PEAK - SEA_LEVEL) + SEA_LEVEL;

            //height_map[i + k * Chunk::CHUNK_SIZE] = (noise.GetNoise(float(world_x_pos), float(world_z_pos)) + 1) * Chunk::CHUNK_SIZE * COLUMN_HEIGHT * 0.5f;
        }
    }
    */
}

void Column::draw(Shader& shader) {
    for (Chunk& chunk : chunks) {
        chunk.draw(shader);
    }
}

void Column::generateVertices() {
    for (Chunk& chunk : chunks) {
        chunk.generateVertices();
    }
}

void Column::generateMeshes() {
    for (Chunk& chunk : chunks) {
        chunk.generateMesh();
    }
}

void Column::refreshMeshes() {
    for (Chunk& chunk : chunks) {
        chunk.refreshMesh();
    }
}

void Column::destroyMeshes() {
    for (Chunk& chunk : chunks) {
        chunk.destroyMesh();
    }
}

FastNoiseLite Column::noise;

void Column::configureNoiseGenerator() {
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(1337);
    noise.SetFrequency(0.01);

    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(3);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.4f);
}