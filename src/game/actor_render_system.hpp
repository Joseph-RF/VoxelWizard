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
};