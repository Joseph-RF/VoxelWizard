#include "actor_control_system.hpp"
#include "actor_control_system.hpp"

#include "engine/ecs/coordinator.hpp"
#include "game/components.hpp"

#include <glm/gtc/matrix_transform.hpp>

extern Coordinator g_coordinator;

void ActorControlSystem::init() {
}

void ActorControlSystem::push_action(Action action) {
	m_actions.push(action);
}

void ActorControlSystem::update(float dt, const glm::vec3& camera_view, const glm::vec3& camera_up) {

	// Determine forward and right directions
	glm::vec3 forward = glm::normalize(glm::vec3(camera_view.x, 0.f, camera_view.z));
	glm::vec3 right = glm::normalize(glm::cross(forward, camera_up));

	while (!m_actions.empty()) {
		for (auto const& entity : m_entities) {
			auto& transform = g_coordinator.getComponent<Transform>(entity);
			auto& controllable = g_coordinator.getComponent<Controllable>(entity);

			switch (m_actions.front()) {
			case Action::MOVE_FORWARD:
				transform.position += forward * controllable.movement_speed * dt;
				break;
			case Action::MOVE_BACKWARD:
				transform.position -= forward * controllable.movement_speed * dt;
				break;
			case Action::MOVE_LEFT:
				transform.position -= right * controllable.movement_speed * dt;
				break;
			case Action::MOVE_RIGHT:
				transform.position += right * controllable.movement_speed * dt;
				break;
			}
		}
		m_actions.pop();
	}
}
