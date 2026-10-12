#pragma once

#include "world_objects.hpp"

class World {
public:
    void init();

    void render();

    std::vector<Column>& getColumns(); 
    Column& getColumn(ColumnPos column_pos);
    Chunk& getChunk(ChunkPos chunk_pos);

    void updateColumnEdgeOccupancy(Column& column);
    void updateChunkEdgeOccupancy(Chunk& chunk);
private:
    std::vector<Column> columns;
};

// Game data functions
static ChunkPos getChunkPos(int x_pos, int y_pos, int z_pos);
static ChunkPos getChunkPos(glm::vec3 pos);
static inline int getColumnIndex(ColumnPos column_pos);
static inline bool inBounds(ColumnPos column_pos);