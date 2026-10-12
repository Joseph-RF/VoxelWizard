#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <bitset>

// Simple id used to identify something in the game such as the player or enemies
using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 500;

// Each COMPONENT such as being a RigidBody or Controllable will have a uniqud id
using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

// Will use SIGNATURE to keep track of the COMPONENTS an ENTITY is associated with
using Signature = std::bitset<MAX_COMPONENTS>;

namespace Voxel {
static constexpr int WORLD_SIZE = 32;
static constexpr int HALF_WORLD_SIZE = 16;
static constexpr int WORLD_SIZE_SQ = WORLD_SIZE * WORLD_SIZE;

static constexpr float BLOCK_LENGTH = 1.0f;

static constexpr int CHUNK_SIZE = 16;
static constexpr int CHUNK_SIZE_SQ = CHUNK_SIZE * CHUNK_SIZE;
static constexpr int CHUNK_SIZE_CB = CHUNK_SIZE * CHUNK_SIZE_SQ;
};