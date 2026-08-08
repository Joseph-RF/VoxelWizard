#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <future>
#include <chrono>
#include <utility>
#include <memory>

#include "camera.hpp"
#include "glfwwindowmanager.hpp"
#include "renderer.hpp"
#include "game_objects.hpp"

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

    // Game data
    static constexpr int WORLD_SIZE = 32;
    static constexpr int HALF_WORLD_SIZE = 16;
    static constexpr int WORLD_SIZE_SQ = WORLD_SIZE * WORLD_SIZE;
    glm::vec2 world_centre_pos{ WORLD_SIZE * Chunk::CHUNK_SIZE * Block::side_length / 2.f,
                                WORLD_SIZE * Chunk::CHUNK_SIZE * Block::side_length / 2.f };
    std::vector<Column> columns;
    ColumnPos current_column_pos{ 10, 10 };

    // ----- Concurrency -----

    std::future<void> helper_thread;
    std::future_status helper_thread_status;

    // Initialising functions
    bool init();

    void initScene();

    // Updating functions
    void update();
    void updateColumnEdgeOccupancy(Column& column);

    void runActions();

    // Rendering functions
    void render();

    void renderImGUI();

    // Pseudo updating functions
    void processMouseMovement(float mouse_xpos, float mouse_ypos);

    void processScreenResize(float new_window_x, float new_window_y);

    // Game data functions
    ChunkPos getChunkPos(int x_pos, int y_pos, int z_pos);
    ChunkPos getChunkPos(glm::vec3 pos);
    inline int getColumnIndex(ColumnPos column_pos);
    inline bool inBounds(ColumnPos column_pos);

    // Test code
    void initSceneTEST();
};
