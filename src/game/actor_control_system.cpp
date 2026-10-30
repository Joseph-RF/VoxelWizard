#include "actor_control_system.hpp"
#include "actor_control_system.hpp"

#include "engine/ecs/coordinator.hpp"
#include "game/components.hpp"

#include <glm/gtc/matrix_transform.hpp>

extern Coordinator g_coordinator;

void ActorControlSystem::init(std::shared_ptr<Camera> camera) {
	m_camera = camera;
}

void ActorControlSystem::push_action(Action action) {
	m_actions.push(action);
}

void ActorControlSystem::update(float dt, const glm::vec2& mouse_position) {

	setTargetOrientation(mouse_position);

	executeActions(dt);
}

void ActorControlSystem::setTargetOrientation(const glm::vec2& mouse_position) {
	// Have the controllable entities face where the mouse is
	// Find what point at y=0 the mouse points at
	glm::vec3 mouse_position_world = m_camera->windowToWorldCoords(mouse_position);

	// Assume that entities have zero rotation around y axis when they are facing in +x direction
	// Will call this "north"
	glm::vec3 north = glm::vec3(1, 0, 0);

	for (auto const& entity : m_entities) {
		auto& transform = g_coordinator.getComponent<Transform>(entity);

		glm::vec3 entity_to_mouse = mouse_position_world - transform.position;
		entity_to_mouse = glm::vec3(entity_to_mouse.x, 0, entity_to_mouse.z);

		float denom = glm::length(north) * glm::length(entity_to_mouse);
		float angle = glm::acos(glm::dot(north, entity_to_mouse) / denom);

		// Check to see which way the angle winds
		if (glm::cross(north, entity_to_mouse).y < 0) {
			transform.rotation.y = 2 * glm::pi<float>() - angle;
		}
		else {
			transform.rotation.y = angle;
		}
	}
}

void ActorControlSystem::executeActions(float dt) {
	glm::vec3 camera_view = m_camera->front - m_camera->pos;
	glm::vec3 camera_up = m_camera->world_space_up;

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
