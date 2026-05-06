#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    Camera* active_camera;
    Camera engine_camera;
    Camera raytracer_camera;

    // Renderer
    Renderer renderer;

    // Game data
    std::unordered_map<ChunkPos, Chunk, HashFunction> chunks;
    std::vector<ChunkPos> chunks_to_be_rendered;
    int render_distance = 2;
    int mesh_distance = 3;
    int chunk_distance = 4;
    ChunkPos current_chunk_pos;

    // Initialising functions
    bool init();

    void initObjects();
    void initScene();

    // Updating functions
    void update();
    void checkCurrentChunk();
    void updateChunks();
    void updateChunksToBeRendered();

    void updateChunkList();
    void updateMeshList();
    void updateRenderList();

    void updateChunkBlockActivity(ChunkPos chunk_pos);
    void updateBlockActivity(Block& block, int x_pos, int y_pos, int z_pos);
    bool isBlockOpaque(int x_pos, int y_pos, int z_pos);

    void runActions();

    // Rendering functions
    void render();

    void renderImGUI();

    // Pseudo updating functions
    void processMouseMovement(float mouse_xpos, float mouse_ypos);

    void processScreenResize(float new_window_x, float new_window_y);

    // Game data functions
    void createChunks();
    ChunkPos getChunkPos(int x_pos, int y_pos, int z_pos);
    ChunkPos getChunkPos(glm::vec3 pos);
};
