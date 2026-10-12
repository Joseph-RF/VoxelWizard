#pragma once

#include "system.hpp"
#include "engine/core/types.hpp"

#include <memory>
#include <unordered_map>
#include <cassert>

// SystemManager is in charge of maintaining all of the different registered systems and their
// signatures.
// To create a new system, need to call registerSystem which will add the system type to the
// systems map using the typeid name as the key.
// For the program to know which entities belong to each system, need to set a Signature to
// each System.
// Need to create a System, then the appropriate Signature depending on which Components are
// required. Entities will be added to each System through the entitySignatureChanged function

class SystemManager {
public:
	template<typename T>
	std::shared_ptr<T> registerSystem() {
		const char* type_name = typeid(T).name();

		assert(m_systems.find(type_name) == m_systems.end() && "Registering system more than once.");

		// Create a pointer of this system and return it so it can be used externally
		std::shared_ptr<T> system = std::make_shared<T>();
		m_systems.insert({type_name, system});
		return system;
	}

	template<typename T>
	void setSignature(Signature signature) {
		const char* type_name = typeid(T).name();

		assert(m_systems.find(type_name) != m_systems.end() && "System used before being registered.");

		// Set the signature for the system
		m_signatures.insert({ type_name, signature });
	}

	void entityDestroyed(Entity entity) {
		// Remove entity from all system lists
		// Since m_entities in Set is an std::set, don't need to check if entity exists in it
		for (auto const& pair : m_systems) {
			auto const& system = pair.second;

			system->m_entities.erase(entity);
		}
	}

	void entitySignatureChanged(Entity entity, Signature entity_signature) {
		// entity's signature has been changed, whether it has lost or gained a component,
		// systems need to add it to their m_entities or remove from

		// Go through each system
		for (auto const& pair : m_systems) {
			auto const& type_name = pair.first;
			auto const& system = pair.second;
			auto const& system_signature = m_signatures[type_name];


			// Check if the entities new signature matches this current system's
			// If so, add entity to the system's m_entities. If the new signature does
			// not match, remove it from the system
			if ((entity_signature & system_signature) == system_signature) {
				system->m_entities.insert(entity);
			} else {
				system->m_entities.erase(entity);
			}
		}
	}

private:
	// Map going from const char* to Signature
	std::unordered_map<const char*, Signature> m_signatures{};

	// Map going from const char* to a System pointer
	std::unordered_map<const char*, std::shared_ptr<System>> m_systems{};
};