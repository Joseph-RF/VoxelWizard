#pragma once

#include "engine/core/types.hpp"

#include <cassert>
#include <array>
#include <unordered_map>

// This should be the only instance of virtual inheritance in the ECS implementation
// Needed as an interface for ComponentManager class to be able to tell a generic
// ComponentArray that an entity has been destroyed and it needs to update its array mappings.
// This is because we don't know at compile time which COMPONENT, ENTITY will have
// and therefore which COMPONENTS, ENTITY must be removed from.
// Should be ok.

class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	virtual void entityDestroyed(Entity entity) = 0;
};

// ComponentArray<T> will hold an array of components for entities
// Say the component:
// struct RigidBody {
//     vec3 position;
//     vec3 velocity;
// };
// Then ComponentArray<RigidBody> would hold MAX_ENTITIES number of
// RigidBody's in an array

// ComponentArray's primary restriction is that it MUST be compact.
// I.e. there can be no holes in the array, therefore when an entity is removed,
// the gap created by it's component must be filled by the component of an
// existing entity. In this case it will be the last valid entity.

template<typename T>
class ComponentArray : public IComponentArray {
public:
	void insertData(Entity entity, T component) {
		assert(m_entity_to_index_map.find(entity) == m_entity_to_index_map.end() &&
			"Component added to an entity that already has this component");

		// Put new entry at the end and update the maps
		size_t new_index = m_size;
		m_entity_to_index_map[entity] = new_index;
		m_index_to_entity_map[new_index] = entity;
		m_component_array[new_index] = component;
		++m_size;
	}

	// Removing component T from the entity, need to remove said entity from the array
	// Will copy the entity furthest back in the array to the position of the removed entity
	void removeData(Entity entity) {
		assert(m_entity_to_index_map.find(entity) != m_entity_to_index_map.end() &&
			"Entity being removed does not have this component");

		// Copy the last entity to the deleted entity to maintain density
		size_t index_of_removed_entity = m_entity_to_index_map[entity];
		size_t index_of_last_entity = m_size - 1;
		m_component_array[index_of_removed_entity] = m_component_array[index_of_last_entity];

		// Update the map at where the last entity is now (where the removed entity was)
		Entity last_entity = m_index_to_entity_map[index_of_last_entity];
		m_entity_to_index_map[last_entity] = index_of_removed_entity;
		m_index_to_entity_map[index_of_removed_entity] = last_entity;

		// Removed entity should not map to anything anymore
		m_entity_to_index_map.erase(entity);
		// Index of last element is now vacated and should not point at anything
		m_index_to_entity_map.erase(index_of_last_entity);

		--m_size;
	}

	T& getData(Entity entity) {
		assert(m_entity_to_index_map.find(entity) != m_entity_to_index_map.end() &&
			"Entity does not have this component");

		return m_component_array[m_entity_to_index_map[entity]];
	}

	void entityDestroyed(Entity entity) override {
		if (m_entity_to_index_map.find(entity) != m_entity_to_index_map.end()) {

			// If entity exists, remove it's entity
			removeData(entity);
		}
	}

private:
	std::array<T, MAX_ENTITIES> m_component_array;

	// Maps entity id to an array index for m_component_array
	std::unordered_map<Entity, size_t> m_entity_to_index_map;

	// Maps from an array index for m_component_array to the entity id
	std::unordered_map<size_t, Entity> m_index_to_entity_map;

	// Total number of VALID entries in the array
	size_t m_size{};
};