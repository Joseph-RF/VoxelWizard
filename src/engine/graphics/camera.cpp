#include "camera.hpp"

Camera::Camera(const int& WORLD_SIZE, const int& CHUNK_SIZE, const float& block_side_lenth) {
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

	mouse_sensitivity = 0.1f;
	scroll_sensitivity = 1.0f;
}

void Camera::processKeyboard(Camera_Movement direction, float delta_time) {
	
	float orbit_angle_rads = glm::radians(orbit_angle);

	glm::vec3 displacement;

	if (direction == FORWARD) {
		displacement = speed * delta_time * glm::vec3(-sin(orbit_angle_rads), 0, -cos(orbit_angle_rads));
	}
	else if (direction == LEFT) {
		displacement = speed * delta_time * glm::vec3(-cos(orbit_angle_rads), 0, sin(orbit_angle_rads));
	}
	else if (direction == BACKWARD) {
		displacement = speed * delta_time * glm::vec3(sin(orbit_angle_rads), 0, cos(orbit_angle_rads));
	}
	else if (direction == RIGHT) {
		displacement = speed * delta_time * glm::vec3(cos(orbit_angle_rads), 0, -sin(orbit_angle_rads));
	}

	pos += displacement;
	front += displacement;
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