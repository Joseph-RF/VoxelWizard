#pragma once

#include "engine/ecs/system.hpp"

#include "engine/input/events.hpp"
#include "engine/graphics/camera.hpp"

#include <glm/glm.hpp>

#include <memory>

class ActorControlSystem : public System {
public:
    void init(std::shared_ptr<Camera> camera);

    void push_action(Action action);

    void update(float dt, const glm::vec2& mouse_position);

private:
    void setTargetOrientation(const glm::vec2& mouse_position);
    void executeActions(float dt);
    
    std::queue<Action> m_actions{};
    std::shared_ptr<Camera> m_camera;
};