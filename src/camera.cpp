#include <camera.hpp>

Camera::Camera() {
	speed = 5.0f;
	pos = glm::vec3(0.0f, 0.0f, 3.0f);
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	fov = 45.0f;

	yaw = -90.f;
	pitch = 0.0;

	mouse_sensitivity = 0.1f;
	scroll_sensitivity = 1.0f;
}

void Camera::processKeyboard(Camera_Movement direction, float delta_time) {
	if (direction == FORWARD) {
		pos += speed * delta_time * front;
	}
	else if (direction == LEFT) {
		// To get side stepping motion, need to multiply camera_speed by "right" or "left"
		// Find those vectors by doing a cross product of the "forward" facing vector
		// by the "upwards" facing vector
		pos -= glm::normalize(glm::cross(front, up)) * speed * delta_time;
	}
	else if (direction == BACKWARD) {
		pos -= speed * delta_time * front;
	}
	else if (direction == RIGHT) {
		pos += glm::normalize(glm::cross(front, up)) * speed * delta_time;
	}
	else if (direction == UP) {
		pos += glm::normalize(glm::cross(glm::cross(front, up), front)) * speed * delta_time;
	}
	else if (direction == DOWN) {
		pos -= glm::normalize(glm::cross(glm::cross(front, up), front)) * speed * delta_time;
	}
}

void Camera::processMouse(double x_offset, double y_offset) {
	yaw += mouse_sensitivity * x_offset;
	pitch += mouse_sensitivity * -y_offset;

	// Add a check to ensure pitch doesn't go so far as to invert look up direction
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	float x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	float y = sin(glm::radians(pitch));
	float z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(glm::vec3(x, y, z));
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

	return glm::lookAt(pos, front + pos, up);
}