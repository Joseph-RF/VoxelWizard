#include "camera.hpp"

#include "engine/ecs/coordinator.hpp"
#include "game/components.hpp"

#include <iostream>

extern Coordinator g_coordinator;

Camera::Camera(const int& WORLD_SIZE, const int& CHUNK_SIZE, const float& block_side_lenth, float window_x, float window_y) {
	ortho = true;
	zoom = 0.38f;

	orbit_radius = WORLD_SIZE * CHUNK_SIZE * block_side_lenth;
	orbit_angle = 0.f;
	yaw = 0.f;
	pitch = 30.f;
	speed = 100.0f;

	// Calculate height from orbit radius and desired yaw
	orbit_height = orbit_radius * tan(glm::radians(pitch));

	pos = glm::vec3(orbit_radius * sin(glm::radians(orbit_angle)),
		orbit_height,
		orbit_radius * cos(glm::radians(orbit_angle))
	);
	// Camera will start by pointing at origin
	front = glm::vec3{ 0, 0, 0 };

	world_space_up = glm::vec3(0.0f, 1.0f, 0.0f);
	fov = 45.0f;
	near_plane = 0.1f;
	far_plane = 10000.f;

	view = lookAt();
	if (ortho) {
		projection = glm::ortho(-window_x * zoom / 2.f,
								window_x * zoom / 2.f,
								-window_y * zoom / 2.f,
								window_y * zoom / 2.f,
								near_plane,
								far_plane
		);
	}
	else {
		projection = glm::perspective(
			glm::radians(fov), (window_size.x / window_size.y), near_plane, far_plane
		);
	}

	mouse_sensitivity = 0.1f;
	scroll_sensitivity = 1.0f;

	window_size = glm::vec2(window_x, window_y);

	deadzone_minima = window_size * deadzone_ratio;
	deadzone_maxima = window_size * (1.0f - deadzone_ratio);
}

void Camera::processMouse(double x_offset, double y_offset) {
	
	// Will reverse mouse offset out of preference of how the world rotates with mouse movement
	yaw += mouse_sensitivity * (-x_offset);
	orbit_angle = yaw;

	pos = glm::vec3(front.x + orbit_radius * sin(glm::radians(orbit_angle)),
		orbit_height,
		front.z + orbit_radius * cos(glm::radians(orbit_angle))
	);
}

void Camera::processScroll(double x_offset, double y_offset) {

	fov += -static_cast<float>(y_offset) * scroll_sensitivity;

	if (fov > 140.0f) {
		fov = 140.0f;
	}
	if (fov < 5.0f) {
		fov = 5.0f;
	}
}

glm::mat4 Camera::lookAt()
{
	// Argument 1: Position of the camera. NOTE THAT THIS IS REVERSED ON PURPOSE
	// Argument 2: Direction camera is looking in.
	// Argument 3: Up direction of the camera
	// The "right" of the camera aim will be calculated through cross product of 2 and 3

	return glm::lookAt(pos, front, world_space_up);
}

void Camera::update() {
	// Calculate view and projection
	view = lookAt();
	if (ortho) {
		projection = glm::ortho(-window_size.x * zoom / 2.f,
								window_size.x * zoom / 2.f,
								-window_size.y * zoom / 2.f,
								window_size.y * zoom / 2.f,
								near_plane,
								far_plane
		);
	}
	else {
		projection = glm::perspective(
			glm::radians(fov), (window_size.x / window_size.y), near_plane, far_plane
		);
	}

	checkTargetPosition();
}

void Camera::setTargetEntity(Entity target) {
	this->target_entity = target;
}

Entity Camera::getTargetEntity() {
	return target_entity;
}

void Camera::processScreenResize(float window_x, float window_y) {
	window_size = glm::vec2(window_x, window_y);
	deadzone_minima = window_size * (0.5f - deadzone_ratio);
	deadzone_maxima = window_size * (1.0f - deadzone_ratio);
}

glm::vec2 Camera::worldToWindowCoords(const glm::vec3& world_position) {
	// Calculate world position in screen coordinates
	glm::vec4 clip_space = projection * view * glm::vec4(world_position, 1.0f);
	glm::vec3 ndc_space;

	if (clip_space.w == 0.f) {
		std::cout << "Attempt to divide by zero in worldToScreenCoords function call." << std::endl;
		return glm::vec2(0.f, 0.f);
	}

	ndc_space = glm::vec3(clip_space.x, clip_space.y, clip_space.z) / clip_space.w;

	return glm::vec2(ndc_space.x + 1.f, 1.f - ndc_space.y) * 0.5f * window_size;
}

glm::vec3 Camera::windowToWorldCoords(const glm::vec2& window_position) {
	// Will take a 2D window_position, and will return a 3D world-position
	// Achieved by shooting a ray from the very front of the frustum and finding
	// the y=0 intercept (or wherever the ground is)
	
	// Convert to NDC space
	glm::vec3 ndc_space((2.0f * window_position.x - window_size.x) / window_size.x,
						(window_size.y - 2.0f * window_position.y) / window_size.y,
						1.0f);

	// Don't need to divide since the ray's length doesn't matter
	glm::vec4 clip_space_near(ndc_space.x, ndc_space.y, -1.0f, 1.0f);
	glm::vec4 clip_space_far(ndc_space.x, ndc_space.y, 1.0f, 1.0f);

	glm::mat4 inverse_proj_view = glm::inverse(projection * view);

	glm::vec4 ray_near = inverse_proj_view * clip_space_near;
	ray_near /= ray_near.w;
	glm::vec3 ray_origin(ray_near.x, ray_near.y, ray_near.z);

	glm::vec4 ray_far = inverse_proj_view * clip_space_far;
	ray_far /= ray_far.w;

	// Use camera's front - position as the direction
	glm::vec3 ray_direction = glm::normalize(glm::vec3(ray_far.x, ray_far.y, ray_far.z) - glm::vec3(ray_near.x, ray_near.y, ray_near.z));

	if (ray_direction.y == 0) {
		std::cout << "Parellel to plane. This should not happen since 2.5iso" << std::endl;
	}

	// Find the intercept with y=0 plane
	float t = -(ray_near.y) / ray_direction.y;

	if (t < 0) {
		std::cout << "Behind the plane. Again, should not happen due to the inclination of the world with respect to the camera" << std::endl;
	}

	return ray_origin + t * ray_direction;
}

void Camera::checkTargetPosition() {
	// Check if the target entity has moved outside of the deadzone.
	// Update camera accordingly
	glm::vec3 target_position_world = g_coordinator.getComponent<Transform>(target_entity).position;
	glm::vec2 target_position_window = worldToWindowCoords(target_position_world);
	glm::vec2 deadzone_diff_window(0.f, 0.f);

	if (target_position_window.x > deadzone_maxima.x) {
		deadzone_diff_window.x = target_position_window.x - deadzone_maxima.x;
	}
	else if (target_position_window.x < deadzone_minima.x) {
		deadzone_diff_window.x = target_position_window.x - deadzone_minima.x;
	}
	if (target_position_window.y > deadzone_maxima.y) {
		deadzone_diff_window.y = target_position_window.y - deadzone_maxima.y;
	}
	else if (target_position_window.y < deadzone_minima.y) {
		deadzone_diff_window.y = target_position_window.y - deadzone_minima.y;
	}

	if (deadzone_diff_window.x == 0.f && deadzone_diff_window.y == 0.f) {
		return;
	}

	// Target is outside of the deadzone

	glm::vec2 new_camera_objective_window = window_size * 0.5f + deadzone_diff_window;
	glm::vec3 new_camera_objective_world = windowToWorldCoords(new_camera_objective_window);
	glm::vec3 centre_world = windowToWorldCoords(window_size * 0.5f);

	glm::vec3 camera_push_world = new_camera_objective_world - centre_world;

	front += camera_push_world;
	pos += camera_push_world;

	// Recalculate view with new front and pos
	view = lookAt();
}
