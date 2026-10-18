#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct Renderable {
    uint16_t mesh_id;
    uint16_t shader_id;
};

struct Controllable {
    float movement_speed;
};