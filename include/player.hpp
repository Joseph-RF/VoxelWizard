#pragma once

#include <glm/glm.hpp>

#include <string>

#include "mesh.hpp"
#include "shader.hpp"

struct Player {
	Player();
	Player(glm::vec3 position, float rotation_y);

	void draw(const Mesh& mesh, Shader& shader);

	void setMeshID(const std::string& mesh_id) { this->mesh_id = mesh_id; };

	glm::vec3 position;
	float rotation_y;
	float size = 1.f;

	std::string mesh_id;
};