#include <player.hpp>

Player::Player() {
    position = glm::vec3(0, 10, 0);
    rotation_y = 0.f;

    mesh_id = "default";
}

Player::Player(glm::vec3 position, float rotation_y) : position(position), rotation_y(rotation_y) {
    mesh_id = "default";
}

void Player::draw(const Mesh& mesh, Shader& shader) {

    glBindVertexArray(mesh.vao);
    shader.use();

    glm::mat4 model(1.0);
    model = glm::translate(model, position);
    model = glm::scale(model, size * glm::vec3(1));
    shader.setMat("model", model);

    if (mesh.mesh_mode == MeshMode::INDEXED) {
        glDrawElements(GL_TRIANGLES, mesh.triangle_count * 3, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}