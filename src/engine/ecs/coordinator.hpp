#pragma once

#include "entity_manager.hpp"
#include "component_manager.hpp"
#include "system_manager.hpp"

class Coordinator {
public:
	void init() {
		m_entity_manager = std::make_unique<EntityManager>();
		m_component_manager = std::make_unique<ComponentManager>();
		m_system_manager = std::make_unique<SystemManager>();
	}

	// Entity methods:
	Entity createEntity() {
		return m_entity_manager->createEntity();
	}

	void destroyEntity(Entity entity) {
		m_entity_manager->destroyEntity(entity);
		m_component_manager->entityDestroyed(entity);
		m_system_manager->entityDestroyed(entity);
	}

	// Component methods:
	template<typename T>
	void registerComponent() {
		m_component_manager->registerComponent<T>();
	}

	template<typename T>
	void addComponent(Entity entity, T component) {
		m_component_manager->addComponent<T>(entity, component);

		// Get the entity's current signature to update with the new signature
		Signature signature = m_entity_manager->getSignature(entity);
		signature.set(m_component_manager->getComponentType<T>(), true);
		m_entity_manager->setSignature(entity, signature);
		
		m_system_manager->entitySignatureChanged(entity, signature);
	}

	template<typename T>
	void removeComponent(Entity entity) {
		m_component_manager->removeComponent<T>(entity);

		Signature signature = m_entity_manager->getSignature(entity);
		signature.set(m_component_manager->GetComponentType<T>(), false);
		m_entity_manager->setSignature(entity, signature);

		m_system_manager->entitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& getComponent(Entity entity) {
		return m_component_manager->getComponent<T>(entity);
	}

	template<typename T>
	ComponentType getComponentType() {
		return m_component_manager->getComponentType<T>();
	}

	// System methods:
	template<typename T>
	std::shared_ptr<T> registerSystem() {
		return m_system_manager->registerSystem<T>();
	}

	template<typename T>
	void setSystemSignature(Signature signature) {
		m_system_manager->setSignature<T>(signature);
	}

private:
	std::unique_ptr<EntityManager> m_entity_manager;
	std::unique_ptr<ComponentManager> m_component_manager;
	std::unique_ptr<SystemManager> m_system_manager;
};