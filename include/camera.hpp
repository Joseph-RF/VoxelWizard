#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum Camera_Movement {
	FORWARD,
	LEFT,
	BACKWARD,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	Camera() = delete;
	Camera(const int& WORLD_SIZE, const int& CHUNK_SIZE, const float& block_side_lenth);

	void processKeyboard(Camera_Movement direction, float delta_time);
	void processMouse(double x_offset, double y_offset);
	void processScroll(double x_offset, double y_offset);
	glm::mat4 lookAt();

	// Camera values
	float speed;
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 world_space_up;
	float fov;

	float yaw;
	float pitch;

	float orbit_radius;
	float orbit_angle; // Curling around y axis, starting at positive x moving to positive z
	float orbit_height;

	float mouse_sensitivity;
	float scroll_sensitivity;
};