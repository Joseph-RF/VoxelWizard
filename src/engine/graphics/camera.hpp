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
	Camera(float window_x, float window_y);

	void processMouse(double x_offset, double y_offset);
	void processScroll(double x_offset, double y_offset);
	void turnLeft();
	void turnRight();
	glm::mat4 lookAt();

	void update();

	void setTargetEntity(Entity target);
	Entity getTargetEntity();

	void processScreenResize(float window_x, float window_y);

	glm::vec2 worldToWindowCoords(const glm::vec3& world_position);
	glm::vec3 windowToWorldCoords(const glm::vec2& window_position);

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

private:
	void checkTargetPosition();
	void shouldRotate();
	void shouldFollowTarget();

	float deadzone_ratio{ 0.3f };
	glm::vec2 deadzone_minima;
	glm::vec2 deadzone_maxima;
	glm::vec2 deadzone_padding;

	bool turning_left;
	bool turning_right;
	float max_turning_speed;
	float target_angle;

	bool translating;
	float new_front_distance; // Scalar distance camera must move to reach new front position
	float distance_traveled; // Scalar distance camera has traveled so far to reach current new front
	glm::vec2 normalised_translation_direction;

	Entity target_entity;
	glm::vec2 window_size;
};