#pragma once

#include "engine/core/types.hpp"

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
	Camera(const int& WORLD_SIZE, const int& CHUNK_SIZE, const float& block_side_lenth, float window_x, float window_y);

	void processMouse(double x_offset, double y_offset);
	void processScroll(double x_offset, double y_offset);
	glm::mat4 lookAt();

	void update();

	void setTargetEntity(Entity target);
	Entity getTargetEntity();

	void processScreenResize(float window_x, float window_y);

	// Camera values
	bool ortho;
	float zoom;

	float speed;
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 world_space_up;
	float fov;
	float near_plane;
	float far_plane;

	glm::mat4 view;
	glm::mat4 projection;

	float yaw;
	float pitch;

	float orbit_radius;
	float orbit_angle; // Curling around y axis, starting at positive x moving to positive z
	float orbit_height;

	float mouse_sensitivity;
	float scroll_sensitivity;

	float deadzone_ratio{ 0.3f };
	glm::vec2 deadzone_minima;
	glm::vec2 deadzone_maxima;

private:
	glm::vec2 worldToWindowCoords(const glm::vec3& world_position);
	glm::vec3 windowToWorldCoords(const glm::vec2& window_position);

	void checkTargetPosition();

	Entity target_entity;
	glm::vec2 window_size;
};