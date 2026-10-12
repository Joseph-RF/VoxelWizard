#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct Renderable {
    uint32_t mesh_id;
    uint32_t shader_id;
};