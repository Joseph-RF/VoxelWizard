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
    std::unordered_map<ChunkStackPos, ChunkStack, ChunkStackPosHashFunc> chunks;
    std::vector<ChunkStackPos> chunk_stacks_to_be_rendered;
    int render_distance = 4;
    int mesh_distance = 4;
    int chunk_distance = 4;
    //ChunkPos current_chunk_pos{10, 0, 10};
    ChunkStackPos current_chunk_stack_pos{ 10, 10 };

    // ----- Concurrency -----
    // Queues of things that need to be created
    std::queue<ChunkStackPos> chunk_stack_creation_queue_main; // Chunk stacks that need to be created. For main thread
    std::queue<ChunkStackPos> chunk_stack_creation_queue_helper; // List of chunk stacks to be created sent to the helper thread
    std::vector<ChunkStack> helper_created_chunk_stacks; // Chunk stacks created by the helper thread to be passed to main thread
    std::unordered_map<ChunkStackPos, int, ChunkStackPosHashFunc> chunk_stacks_on_helper_thread; // Chunk stacks yet to go to hash map

    std::queue<ChunkStackPos> chunk_stacks_vertex_creation_queue_main; // Chunk stacks whose vertices need to be created. For main thread
    std::queue<ChunkStackPos> chunk_stacks_vertex_creation_queue_helper; // List of chunk stacks who need vertices created sent to helper
    std::queue<ChunkStackPos> stale_chunk_stacks_vertices_helper; // Queue of chunk stacks whose vertices need to be updated

    std::queue<ChunkStackPos> chunk_stack_mesh_creation_queue; // Chunk stacks where chunk meshes are to be created by the main thread
    std::queue<ChunkStackPos> stale_mesh_creation_queue; // Queue of chunk stacks whose chunk meshes need to be re-made

    std::queue<ChunkStackPos> chunk_stack_deletion_queue; // Chunk stacks that need to be deleted. Run when helper thread is sleeping
    std::queue<ChunkStackPos> chunk_stack_mesh_deletion_queue; // Chunk stack whose chunk meshes that need to be deleted. Ditto above.

    std::future<void> helper_thread;
    std::future_status helper_thread_status;

    // Initialising functions
    bool init();

    void initScene();

    // Updating functions
    void update();
    void checkCurrentChunk();
    void updateChunkStackEdgeOccupancy(ChunkStack& chunk_stack);

    void updateChunkQueues();
    void updateChunkStackCreationQueue();
    void updateChunkStackMeshCreationQueues();

    // Run through creation queues
    void swapCreationQueues();
    void processCreationQueues(); // Run by helper thread
    void refreshStaleChunkStackVertices(); // Run by helper thread
    void generateChunkStacks(); // Run by helper thread
    void generateChunkStackVertices(); // Run by helper thread

    void integrateGeneratedChunkStacks(); // Run by main thread
    void swapBuffersAndGenerateMeshes(); // Run by main thread

    // Run through deletion queues
    void processDeletionQueues(); // Run by main thread

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
    ChunkPos getChunkPos(int x_pos, int y_pos, int z_pos);
    ChunkPos getChunkPos(glm::vec3 pos);
};
