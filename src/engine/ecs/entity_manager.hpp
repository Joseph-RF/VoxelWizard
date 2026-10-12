#pragma once

#include "engine/core/types.hpp"

#include <queue>
#include <array>
#include <cassert>

/*
* Entity Manager class. Will use to keep track of which ENTITY ids are in use
* and which are not. A queue will be used to dispense ENTITY codes. Once an entity
* is destroyed and is available again, the id goes to the back of the queue
* 
* Note that setSignature and getSignature don't REQUIRE createEntity or destroyEntity
* to have been called or not
*/

class EntityManager {
public:
	EntityManager() {
		// Initialise the queue with all possible entity id values
		for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
			m_available_entities.push(entity);
		}
	}

	Entity createEntity() {
		assert(m_living_entity_count < MAX_ENTITIES && "Too many entities in existence");

		// Take the first id up to be used
		Entity id = m_available_entities.front();
		m_available_entities.pop();
		++m_living_entity_count;

		return id;
	}

	void destroyEntity(Entity entity) {
		assert(entity < MAX_ENTITIES && "Entity is out of range (greater than MAX_ENTITIES)");

		// Wipe the entities signatures. Id no longer associated with any components
		m_signatures[entity].reset();

		// Put the destroyed id to the back of the queue to be used again
		m_available_entities.push(entity);
		--m_living_entity_count;
	}

	void setSignature(Entity entity, Signature signature) {
		assert(entity < MAX_ENTITIES && "Entity is out of range (greater than MAX_ENTITIES)");

		m_signatures[entity] = signature;
	}

	Signature getSignature(Entity entity) {
		assert(entity < MAX_ENTITIES && "Entity is out of range (greater than MAX_ENTITIES)");

		return m_signatures[entity];
	}

private:
	// Queue of unused entity ids
	std::queue<Entity> m_available_entities{};

	// Array of signatures where the index of each element corresponds to the id of ENTITY
	// that has that signature
	std::array<Signature, MAX_ENTITIES> m_signatures;

	// Total number of live entities, used to keep a limit on the number that exist
	uint32_t m_living_entity_count{};
};