#pragma once

#include "engine/ecs/system.hpp"

#include "engine/input/events.hpp"

#include <glm/glm.hpp>

class ActorControlSystem : public System {
public:
    void init();

    void push_action(Action action);

    void update(float dt, const glm::vec3& camera_view, const glm::vec3& camera_up);

private:
    std::queue<Action> m_actions{};
};