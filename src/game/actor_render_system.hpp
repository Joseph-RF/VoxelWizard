#pragma once

#include "engine/ecs/system.hpp"
#include "engine/core/resource_manager.hpp"

#include <memory>

class ActorRenderSystem : public System {
public:    
    void init(std::shared_ptr<ResourceManager> resource_manager);

    void render();

private:
    std::shared_ptr<ResourceManager> m_resource_manager;

    static constexpr glm::vec3 orientation[3]{
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f)
    };
};