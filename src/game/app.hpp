#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <future>
#include <chrono>
#include <utility>
#include <memory>

#include "engine/graphics/camera.hpp"
#include "engine/graphics/renderer.hpp"
#include "engine/core/glfwwindow_manager.hpp"
#include "engine/core/resource_manager.hpp"

#include "game/world/world_objects.hpp"
#include "game/world/world.hpp"

#include "actor_control_system.hpp"

#include "engine/core/types.hpp"

class App {
public:
    App(int window_x, int window_y);

    ~App();

    void run();

private:
    // Window properties
    int window_x;
    int window_y;
    std::unique_ptr<IWindowManager> window_manager;
    std::shared_ptr<EventManager> event_manager;

    // Timings
    float delta_time;
    float last_frame;

    // Mouse
    bool first_mouse_movement;
    float last_mouse_xpos;
    float last_mouse_ypos;
    bool mouse_pressed;

    // Camera
    Camera camera;

    // Renderer
    Renderer renderer;

    // Resource Manager
    std::shared_ptr<ResourceManager> resource_manager;

    // Systems held by app
    std::shared_ptr<ActorControlSystem> actor_control_system;

    // Game data
    World world;
    glm::vec2 world_centre_pos{ Voxel::WORLD_SIZE * Voxel::CHUNK_SIZE * Voxel::BLOCK_LENGTH / 2.f,
                                Voxel::WORLD_SIZE * Voxel::CHUNK_SIZE * Voxel::BLOCK_LENGTH / 2.f };
    ColumnPos current_column_pos{ 10, 10 };

    // ----- Concurrency -----

    std::future<void> helper_thread;
    std::future_status helper_thread_status;

    // Initialising functions
    bool init();

    void initCoordinator();
    void initSystems();
    void initResources();
    void initWorld();
    void initEntities();

    // Updating functions
    void update();

    void processEvents();

    // Rendering functions
    void render();

    void renderImGUI();

    // Pseudo updating functions
    void processMouseMovement(float mouse_xpos, float mouse_ypos);

    void processScreenResize(float new_window_x, float new_window_y);

    // Test code
    void initSceneTEST();
};
