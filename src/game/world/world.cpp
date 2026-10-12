#include "world.hpp"

#include "engine/core/types.hpp"

#include <iostream>
#include <array>

void World::init() {
    // Set noise generator parameters
    Column::configureNoiseGenerator();

    for (int i = -Voxel::HALF_WORLD_SIZE; i < Voxel::HALF_WORLD_SIZE; ++i) {
        for (int k = -Voxel::HALF_WORLD_SIZE; k < Voxel::HALF_WORLD_SIZE; ++k) {
            columns.emplace_back(ColumnPos(i, k));
        }
    }

    // Update column occupancy data now that they've all been created
    for (Column& column : columns) {
        updateColumnEdgeOccupancy(column);
    }

    // Now that chunk data is ready, generate vertices and meshes
    for (Column& column : columns) {
        column.generateVertices();
        column.generateMeshes();
    }
}

std::vector<Column>& World::getColumns() {
    return columns;
}

void World::updateColumnEdgeOccupancy(Column &column) {
    static const std::array<std::pair<ChunkNeighbour, ChunkPos>, 4> column_neighbour_positions{ {
        {ChunkNeighbour::LEFT, ChunkPos{-1, 0, 0}},
        {ChunkNeighbour::RIGHT, ChunkPos{1, 0, 0}},
        {ChunkNeighbour::BEHIND, ChunkPos{0, 0, -1}},
        {ChunkNeighbour::IN_FRONT, ChunkPos{0, 0, 1}},
    } };

    // Iterate through each individual chunk in a chunk stack for every neighbour direction
    for (const auto& pair : column_neighbour_positions) {
        ColumnPos neighbour_column_pos{
                column.column_pos.x + pair.second.x,
                column.column_pos.z + pair.second.z
        };

        // Check the neighbour is in bounds a.k.a exists
        if (!inBounds(neighbour_column_pos)) { continue; }

        for (Chunk& chunk : column.chunks) {
            chunk.padOccupancy(columns[getColumnIndex(neighbour_column_pos)][chunk.chunk_pos.y], pair.first);
        }
    }

    // Go through the chunks in the chunk stack from the bottom up except for last chunk (nothing above it)
    for (int i = 0; i < Column::COLUMN_HEIGHT - 1; ++i) {
        column.chunks[i].padOccupancy(column.chunks[i + 1], ChunkNeighbour::ABOVE);
    }
    // Go through the chunks in the chunk stack from the top down except for the last chunk (nothing below it)
    for (int i = Column::COLUMN_HEIGHT - 1; i > 0; --i) {
        column.chunks[i].padOccupancy(column.chunks[i - 1], ChunkNeighbour::BELOW);
    }
}

ChunkPos getChunkPos(int x_pos, int y_pos, int z_pos) {
    ChunkPos return_chunk_pos;

    return_chunk_pos.x = (x_pos - (x_pos < 0 ? Voxel::CHUNK_SIZE - 1 : 0)) / Voxel::CHUNK_SIZE;
    return_chunk_pos.y = (y_pos - (y_pos < 0 ? Voxel::CHUNK_SIZE - 1 : 0)) / Voxel::CHUNK_SIZE;
    return_chunk_pos.z = (z_pos - (z_pos < 0 ? Voxel::CHUNK_SIZE - 1 : 0)) / Voxel::CHUNK_SIZE;

    return return_chunk_pos;
}

ChunkPos getChunkPos(glm::vec3 pos) {
    ChunkPos return_chunk_pos(
        std::floor(pos.x / (Voxel::CHUNK_SIZE * Voxel::BLOCK_LENGTH)),
        std::floor(pos.y / (Voxel::CHUNK_SIZE * Voxel::BLOCK_LENGTH)),
        std::floor(pos.z / (Voxel::CHUNK_SIZE * Voxel::BLOCK_LENGTH)));

    return return_chunk_pos;
}

inline int getColumnIndex(ColumnPos column_pos) {

    int x = column_pos.x + Voxel::HALF_WORLD_SIZE;
    int z = column_pos.z + Voxel::HALF_WORLD_SIZE;

    int index = (column_pos.z + Voxel::HALF_WORLD_SIZE) + Voxel::WORLD_SIZE * (column_pos.x + Voxel::HALF_WORLD_SIZE);

    if (index > Voxel::WORLD_SIZE_SQ - 1 || index < 0) {
        std::cout << "Column Index is out of bounds" << std::endl;
        return 0;
    }

    return index;
}

inline bool inBounds(ColumnPos column_pos) {
    if (column_pos.x > Voxel::HALF_WORLD_SIZE - 1 || column_pos.x < -Voxel::HALF_WORLD_SIZE) {
        return false;
    }
    if (column_pos.z > Voxel::HALF_WORLD_SIZE - 1 || column_pos.z < -Voxel::HALF_WORLD_SIZE) {
        return false;
    }
    return true;
}