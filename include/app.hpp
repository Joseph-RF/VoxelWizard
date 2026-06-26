#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <future>
#include <chrono>

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
    int render_distance = 4;
    int mesh_distance = 4;
    int chunk_distance = 6;
    ChunkPos current_chunk_pos{10, 0, 10};

    // ----- Concurrency -----
    // Queues of things that need to be created
    std::queue<ChunkPos> chunk_creation_queue_main; // Chunks that need to be created. For main thread
    std::queue<ChunkPos> chunk_creation_queue_helper; // List of chunks to be created sent to the helper thread
    std::vector<Chunk> helper_created_chunks; // Chunks created by the helper thread to be passed to main thread

    std::queue<ChunkPos> chunk_vertex_creation_queue_main; // Chunks whose vertices need to be created. For main thread
    std::queue<ChunkPos> chunk_vertex_creation_queue_helper; // List of chunks who need vertices created sent to helper

    std::queue<ChunkPos> chunk_mesh_creation_queue; // Chunk meshes to be created by the main thread

    std::queue<ChunkPos> chunk_deletion_queue; // Chunks that need to be deleted. Run when helper thread is sleeping
    std::queue<ChunkPos> chunk_mesh_deletion_queue; // Chunk meshes that need to be deleted. Ditto above.

    std::future<void> helper_thread;
    std::future_status helper_thread_status;

    // Initialising functions
    bool init();

    void initObjects();
    void initScene();

    // Updating functions
    void update();
    void checkCurrentChunk();
    void updateChunks();
    void updateChunkEdgeOccupancy(Chunk& chunk);

    void updateChunkQueues();
    void updateChunkCreationQueue();
    void updateChunkMeshCreationQueues();

    // Run through creation queues
    void swapCreationQueues();
    void consumeCreationQueues(); // Run by helper thread
    void consumeCreatedData(); // Run by main thread

    // Run through deletion queues
    void consumeDeletionQueues(); // Run by main thread

    // Concurrency functions
    bool isHelperThreadFinished();
    void tryHelperThreadLaunch();

    void updateChunksToBeRendered();

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
