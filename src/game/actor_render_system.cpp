#include "actor_render_system.hpp"

#include "engine/ecs/coordinator.hpp"
#include "game/components.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern Coordinator g_coordinator;

void ActorRenderSystem::init(std::shared_ptr<ResourceManager> resource_manager) {
    m_resource_manager = resource_manager;
}

void ActorRenderSystem::render()
{
    for(auto const& entity : m_entities) {
        auto const& transform = g_coordinator.getComponent<Transform>(entity);
        auto const& renderable = g_coordinator.getComponent<Renderable>(entity);

        Mesh* mesh = m_resource_manager->tryGetMesh(renderable.mesh_id);
        Shader& shader = m_resource_manager->getShader(renderable.shader_id);

        if(!mesh) {return;}

        glBindVertexArray(mesh->vao);
        shader.use();

        glm::mat4 model(1.0);
        model = glm::translate(model, transform.position);
        model = glm::scale(model, transform.scale);
        shader.setMat("model", model);

        glDrawElements(GL_TRIANGLES, mesh->triangle_count * 3, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}
