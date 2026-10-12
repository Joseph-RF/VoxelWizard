#pragma once

#include "component_array.hpp"
#include "engine/core/types.hpp"

#include <cassert>
#include <unordered_map>
#include <memory>

// ComponentManager will handle all of the different component arrays when a new component type
// needs to be registered or remove or when an entity is destroyed and its corresponding
// components need to be removed from the appropriate ComponentArrays's
// 
// Will have each type of component have its own UNIQUE bit in the Signature.
// Will use a variable called m_next_component_type that increases by 1 whenever a NEW
// component is registered.
//
// Will use typeid function to return a const char* as a key to the different
// ComponentTypes in a map to get one for each unique component

class ComponentManager {

public:
	template<typename T>
	void registerComponent(){
		const char* type_name = typeid(T).name();

		assert(m_component_types.find(type_name) == m_component_types.end() && "Registering component type more than once.");

		// Add the copmonent type to the component map
		m_component_types.insert({ type_name, m_next_component_type });

		// Create the ComponentArray pointer and add to the map
		m_component_arrays.insert({ type_name, std::make_shared<ComponentArray<T>>() });

		++m_next_component_type;
	}

	template<typename T>
	ComponentType getComponentType() {
		const char* type_name = typeid(T).name();

		assert(m_component_types.find(type_name) != m_component_types.end() && "Component not registered.");

		// Return the component's type (the uint8 used to create the signature) i.e. bit position
		// in the signature
		return m_component_types[type_name];
	}

	template<typename T>
	void addComponent(Entity entity, T component) {
		// entity is having component added to it
		// Need to add an entity to the ComponentArray associated with type T
		getComponentArray<T>()->insertData(entity, component);
	}

	template<typename T>
	void removeComponent(Entity entity) {
		// entity is having component removed from it
		// Need to remove an entity from the ComponentArray associated with type T
		getComponentArray<T>()->removeData(entity);
	}

	template<typename T>
	T& getComponent(Entity entity) {
		// Get the component data belonging to entity
		return getComponentArray<T>()->getData(entity);
	}

	void entityDestroyed(Entity entity) {
		// Need to notify all component arrays that entity has been destroyed
		// ComponentArray does the check to see if it has any data associated with entity
		// and if so, it will remove the data
		for (auto const& pair : m_component_arrays) {
			auto const& component_array = pair.second;
			
			component_array->entityDestroyed(entity);
		}
	}

private:
	// Map from const char* to a component type
	std::unordered_map<const char*, ComponentType> m_component_types{};

	// Map from const char* to a component array
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_component_arrays{};

	// The component type to be assigned to the next newly registered component type, starting at 0
	ComponentType m_next_component_type{};

	// Utility to cast IComponentArray pointer to ComponentArray<T>
	template<typename T>
	std::shared_ptr<ComponentArray<T>> getComponentArray() {
		const char* type_name = typeid(T).name();

		assert(m_component_types.find(type_name) != m_component_types.end() && "Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(m_component_arrays[type_name]);
	}
};