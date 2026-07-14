#include "app.hpp"

#include <unordered_set>

App::App(int window_x, int window_y)
    : renderer(window_x, window_y) {
    // Window properties
    this->window_x = window_x;
    this->window_y = window_y;
    event_manager  = std::make_shared<EventManager>();
    window_manager = makeGLFWWindowManager(window_x, window_y, event_manager);

    // Timings
    delta_time = 0.0f;
    last_frame = 0.0f;

    // Mouse
    first_mouse_movement = true;
    last_mouse_xpos      = window_x / 2.f;
    last_mouse_ypos      = window_y / 2.f;
    mouse_pressed        = false;
}

App::~App() {}

void App::run() {
    if (!init()) {
        // If initialisation failed somehow, quit app
        std::cout << "Initialisation failed" << std::endl;
        return;
    }

    while (!window_manager->shouldWindowClose()) {
        update();

        render();
    }
}

bool App::init() {

    // Create window with the window manager. Window manager responsible for initialising
    // GLFW, OpenGL and ImGUI
    if (!window_manager->createWindow()) {
        std::cout << "Window manager failed to initialise" << std::endl;
        return false;
    }

    renderer.init();

    initScene();

    return true;
}

void App::initScene() {

}

void App::update() {
    float current_frame = window_manager->getTime();
    delta_time          = current_frame - last_frame;
    last_frame          = current_frame;

    runActions();

    // Check if the helper thread is done with its task. If so, do the data swaps.
    // I.e. Add new chunks to the data, swap the vertex and index buffers, generate meshes

    if (isHelperThreadFinished()) {
        integrateGeneratedChunks();
        swapBuffersAndGenerateMeshes();
        processDeletionQueues();
    }

    checkCurrentChunk();
    
    if (!helper_thread.valid() || isHelperThreadFinished()) {
        swapCreationQueues();
    }

    // Attempt to launch the helper thread
    tryHelperThreadLaunch();

    window_manager->update();

    event_manager->update();
}

void App::checkCurrentChunk() {
    ChunkPos chunk_pos = getChunkPos(camera.pos);
    if (!(chunk_pos == this->current_chunk_pos)) {
        this->current_chunk_pos = chunk_pos;
        //this->current_chunk_pos = ChunkPos(0, 0, 0);
        updateChunkQueues();
        updateChunksToBeRendered();
    }
}

void App::updateChunkEdgeOccupancy(Chunk& chunk) {

    static const std::array<std::pair<ChunkNeighbour, ChunkPos>, 6> chunk_neighbour_positions{ {
        {ChunkNeighbour::LEFT, ChunkPos{-1, 0, 0}},
        {ChunkNeighbour::RIGHT, ChunkPos{1, 0, 0}},
        {ChunkNeighbour::BELOW, ChunkPos{0, -1, 0}},
        {ChunkNeighbour::ABOVE, ChunkPos{0, 1, 0}},
        {ChunkNeighbour::BEHIND, ChunkPos{0, 0, -1}},
        {ChunkNeighbour::IN_FRONT, ChunkPos{0, 0, 1}},
    } };

    // Go through the chunk_neighbour positions to pad the occupancy data of chunk
    for (const auto& pair : chunk_neighbour_positions) {
        ChunkPos chunk_pos{ 
            chunk.chunk_pos.x + pair.second.x, 
            chunk.chunk_pos.y + pair.second.y, 
            chunk.chunk_pos.z + pair.second.z 
        };

        // Search in the chunks hash map
        if (chunks.count(chunk_pos) > 0) {
            chunk.padOccupancy(chunks.at(chunk_pos), pair.first);
            // If the neighbour already existed in data, update its occupancy as well
            chunks.at(chunk_pos).padOccupancy(chunk, reverseChunkNeighbour(pair.first));
            
            // Need to update the neighbouring chunk's mesh now
            chunks.at(chunk_pos).setDirty(true);
            stale_chunk_vertices_helper.push(chunk_pos);
        }
        // Search in the chunks that are waiting to be moved to the hash map
        else if (chunks_on_helper_thread.find(chunk_pos) != chunks_on_helper_thread.end()) {
            chunk.padOccupancy(helper_created_chunks[chunks_on_helper_thread.at(chunk_pos)], pair.first);
        }
    }
}

void App::updateChunkQueues() {
    updateChunkCreationQueue();
    updateChunkMeshCreationQueues();
}

void App::updateChunkCreationQueue() {
    std::unordered_set<ChunkPos, HashFunction> chunk_list; // Chunks that SHOULD exist

    // Refresh the current creation queue
    chunk_creation_queue_main = std::queue<ChunkPos>();

    for (int i = -chunk_distance; i <= chunk_distance; ++i) {
        for (int k = -chunk_distance; k <= chunk_distance; ++k) {
            ChunkPos chunk_pos = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);
            if (chunks.count(chunk_pos) == 0) {
                // If a chunk at chunk_pos doesn't exist in memory, add to the creation queue
                chunk_creation_queue_main.push(chunk_pos);
            }
            chunk_list.insert(chunk_pos);
        }
    }

    // Create deletion queue
    for (auto& element : chunks) {
        if (chunk_list.count(element.first) == 0) {
            // If cannot find chunk_pos of current chunk in chunk_list, delete it
            chunk_deletion_queue.push(element.first);
        }
    }
}

void App::updateChunkMeshCreationQueues() {
    // TODO: Will need to modify this when block edition is added.

    // Create list of chunks whose meshes need to be created
    std::unordered_set<ChunkPos, HashFunction> mesh_list;

    // Refresh the current chunk vertex and mesh creation queues
    chunk_vertex_creation_queue_main = std::queue<ChunkPos>();
    chunk_mesh_creation_queue = std::queue<ChunkPos>();

    for (int i = -mesh_distance; i <= mesh_distance; ++i) {
        for (int k = -mesh_distance; k <= mesh_distance; ++k) {
            ChunkPos chunk_pos = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);

            mesh_list.insert(chunk_pos);

            if (chunks.count(chunk_pos) == 0) {
                std::cout << "Chunk missing when vertex creation queue was being updated" << std::endl;
                chunk_creation_queue_main.push(chunk_pos);
                chunk_vertex_creation_queue_main.push(chunk_pos);
                chunk_mesh_creation_queue.push(chunk_pos);
                continue;
            }
            // If a chunk at chunk_pos doesn't have a generated mesh, add to the appropriate creation queues
            if (!chunks.at(chunk_pos).mesh_generated) {
                chunk_vertex_creation_queue_main.push(chunk_pos);
                chunk_mesh_creation_queue.push(chunk_pos);
            }
        }
    }

    // Remove meshes from GPU memory that aren't in the new list of meshes
    for (auto& chunk : chunks) {
        if (mesh_list.count(chunk.first) == 0) {
            chunk_mesh_deletion_queue.push(chunk.first);
        }
    }
}

void App::swapCreationQueues() {
    // Pass queue data from main thread queues to helper thread queues
    chunk_creation_queue_main.swap(chunk_creation_queue_helper);
    std::queue<ChunkPos>().swap(chunk_creation_queue_main); // Empty queue on main

    chunk_vertex_creation_queue_main.swap(chunk_vertex_creation_queue_helper);
    std::queue<ChunkPos>().swap(chunk_vertex_creation_queue_main); // Empty queue on main
}

void App::processCreationQueues() {
    // Create chunks that were in the creation queue as well as the vertices and indices of 
    // chunks that are in the mesh creation queue

    refreshStaleChunkVertices();

    generateChunks();

    generateChunkVertices();
}

void App::refreshStaleChunkVertices() {
    // Update the stale meshes
    while (!stale_chunk_vertices_helper.empty()) {
        ChunkPos chunk_pos = stale_chunk_vertices_helper.front();

        // Don't need to do a dirtied check since it is done in generateVertices()

        // Search in chunks hash map
        if (chunks.count(chunk_pos) > 0) {
            chunks.at(chunk_pos).generateVertices();
            stale_mesh_creation_queue.push(chunk_pos);
        }

        stale_chunk_vertices_helper.pop();
    }
}

void App::generateChunks() {
    while (!chunk_creation_queue_helper.empty()) {
        // Check that the chunk does not already exist anywhere
        ChunkPos chunk_pos = chunk_creation_queue_helper.front();
        if (chunks.count(chunk_pos) == 0 && chunks_on_helper_thread.count(chunk_pos) == 0) {
            helper_created_chunks.emplace_back(chunk_creation_queue_helper.front());
            chunks_on_helper_thread.emplace(chunk_creation_queue_helper.front(), helper_created_chunks.size() - 1);
        }
        chunk_creation_queue_helper.pop();
    }
    // Update the occupancy arrays to reflect occupancy data of neighbouring chunks
    for (Chunk& chunk : helper_created_chunks) {
        updateChunkEdgeOccupancy(chunk);
    }
}

void App::generateChunkVertices() {
    while (!chunk_vertex_creation_queue_helper.empty()) {

        ChunkPos chunk_pos = chunk_vertex_creation_queue_helper.front();

        // Don't need to do a dirtied check since it is done in generateVertices()

        // Search in chunks hash map
        if (chunks.count(chunk_pos) > 0) {
            if (!chunks.at(chunk_pos).mesh_generated && !chunks.at(chunk_pos).vertices_generated) {
                chunks.at(chunk_pos).generateVertices();
            }
        }
        // Search in chunks still sitting in the helper thread array
        else if (chunks_on_helper_thread.count(chunk_pos) > 0) {
            Chunk& chunk = helper_created_chunks[chunks_on_helper_thread.at(chunk_pos)];
            if (!chunk.mesh_generated && !chunk.vertices_generated) {
                chunk.generateVertices();
            }
        }
        chunk_vertex_creation_queue_helper.pop();
    }
}

void App::integrateGeneratedChunks() {
    // Pass chunks constructed by the helper thread to the main chunk hash map
    // Reset the structures used by helper thread to hold generated chunks
    
    for (Chunk& chunk : helper_created_chunks) {
        ChunkPos key = chunk.chunk_pos;
        chunks.try_emplace(key, std::move(chunk));
    }
    std::vector<Chunk>().swap(helper_created_chunks);
    std::unordered_map<ChunkPos, int, HashFunction>().swap(chunks_on_helper_thread);
}

void App::swapBuffersAndGenerateMeshes() {
    // Swap the back and front buffers containing newly made vertex and index data
    // Construct the chunk meshes using the new vertex and index data

    while (!chunk_mesh_creation_queue.empty()) {
        if (!chunks.at(chunk_mesh_creation_queue.front()).mesh_generated) {
            chunks.at(chunk_mesh_creation_queue.front()).swapVertexBuffers();
            chunks.at(chunk_mesh_creation_queue.front()).generateMesh();
        }
        chunk_mesh_creation_queue.pop();
    }

    // Now swap buffers and generate meshes for chunks that had stale vertices and meshes
    while (!stale_mesh_creation_queue.empty()) {
        if (chunks.count(stale_mesh_creation_queue.front()) > 0) {
            chunks.at(stale_mesh_creation_queue.front()).swapVertexBuffers();
            chunks.at(stale_mesh_creation_queue.front()).refreshMesh();
        }
        stale_mesh_creation_queue.pop();
    }

}

void App::processDeletionQueues() {
    // Delete chunks and chunk meshes sent to the deletion queue
    // Check that the chunks exist before deleting them
    while (!chunk_mesh_deletion_queue.empty()) {
        if (chunks.count(chunk_mesh_deletion_queue.front()) > 0) {
            chunks.at(chunk_mesh_deletion_queue.front()).destroyMesh();
        }
        chunk_mesh_deletion_queue.pop();
    }

    while (!chunk_deletion_queue.empty()) {
        if (chunks.count(chunk_deletion_queue.front()) > 0) {
            chunks.erase(chunk_deletion_queue.front());
        }
        chunk_deletion_queue.pop();
    }
}

bool App::isHelperThreadFinished() {
    return helper_thread.valid() &&
        helper_thread.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

void App::tryHelperThreadLaunch() {
    bool finished = isHelperThreadFinished();

    if (!helper_thread.valid() || finished) {
        if (finished) {
            helper_thread.get(); // For syncing and handling execeptions
        }

        helper_thread = std::async(std::launch::async, [this]() {
            processCreationQueues();
        });
    }
}

void App::updateChunksToBeRendered() {
    // TODO: Potentially move the check in renderScene of existing chunks to here instead
    chunks_to_be_rendered = std::vector<ChunkPos>(std::pow(2 * render_distance + 1, 2));
    int counter = 0;
    for (int i = -render_distance; i <= render_distance; i++) {
        for (int k = -render_distance; k <= render_distance; k++) {
            chunks_to_be_rendered[counter] = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);
            counter++;
        }
    }
}

void App::runActions() {
    for (; !event_manager->events.empty(); event_manager->events.pop()) {
        // Consider using a switch statement for the below
        switch (event_manager->events.front().action) {
        case Action::SCREEN_RESIZE:
            processScreenResize(event_manager->events.front().mouse_xpos,
                                event_manager->events.front().mouse_ypos);
            break;
        case Action::MOVE_FORWARD:
            camera.processKeyboard(FORWARD, delta_time);
            break;
        case Action::MOVE_LEFT:
            camera.processKeyboard(LEFT, delta_time);
            break;
        case Action::MOVE_BACKWARD:
            camera.processKeyboard(BACKWARD, delta_time);
            break;
        case Action::MOVE_RIGHT:
            camera.processKeyboard(RIGHT, delta_time);
            break;
        case Action::MOVE_UP:
            camera.processKeyboard(UP, delta_time);
            break;
        case Action::MOVE_DOWN:
            camera.processKeyboard(DOWN, delta_time);
            break;
        case Action::TURN: {
            float mouse_xoffset = event_manager->events.front().mouse_xpos - last_mouse_xpos;
            float mouse_yoffset = event_manager->events.front().mouse_ypos - last_mouse_ypos;
            last_mouse_xpos += mouse_xoffset;
            last_mouse_ypos += mouse_yoffset;

            if (!first_mouse_movement) {
                camera.processMouse(mouse_xoffset, mouse_yoffset);
            } else {
                first_mouse_movement = false;
            }
            break;
        }
        case Action::MOVE_CURSOR: {
            float mouse_xoffset = event_manager->events.front().mouse_xpos - last_mouse_xpos;
            float mouse_yoffset = event_manager->events.front().mouse_ypos - last_mouse_ypos;
            last_mouse_xpos += mouse_xoffset;
            last_mouse_ypos += mouse_yoffset;
            processMouseMovement(last_mouse_xpos, last_mouse_ypos);
            break;
        }
        case Action::ZOOM:
            camera.processScroll(event_manager->events.front().scroll_x,
                                         event_manager->events.front().scroll_y);
            break;
        case Action::L_CLICK:
            // Once a click has been registered, i.e. left mouse button has been released:
            mouse_pressed = false;
            break;
        case Action::L_BUTTON_PRESSED:
            mouse_pressed = true;
            break;
        case Action::R_CLICK:
            std::cout << "Right click registered" << std::endl;
            break;
        case Action::R_BUTTON_PRESSED:
            std::cout << "Right button being held" << std::endl;
            break;
        case Action::TOGGLE_MOUSE:
            window_manager->toggleMouse();
            first_mouse_movement = true;
            break;
        default:
            std::cout << "Action does not have defined behaviour in App::runActions" << std::endl;
        }
    }
}

void App::render() {
    // Let ImGUI know we're working on a new frame
    window_manager->newImGuiFrame();

    // Render scene
    renderer.render(RenderContext{camera, chunks, chunks_to_be_rendered});

    // After drawing OpenGL objects, draw ImGUI
    renderImGUI();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Final step, render to window
    window_manager->renderToWindow();
}

void App::renderImGUI() {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(window_x * 0.15f, window_y));
    ImGui::Begin("ImGUI Window", nullptr);

    ImGui::BeginTabBar("Settings#left_tabs_bar");
    // Object editor tab
    // -------------------------------------
    bool boolean = true;
    if (ImGui::BeginTabItem("Object Editor", &boolean, ImGuiTabItemFlags_None)) {
        ImGui::InputFloat("x pos", &camera.pos.x, 0.25f, 1.0f,
                  "%.2f");
        ImGui::InputFloat("y pos", &camera.pos.y, 0.25f, 1.0f,
          "%.2f");
        ImGui::InputFloat("z pos", &camera.pos.z, 0.25f, 1.0f,
          "%.2f");
        ImGui::Text("Modify the selected objects properties");
        ImGui::EndTabItem();
    }

    // Objects selector tab
    // -------------------------------------
    if (ImGui::BeginTabItem("Objects Selector", &boolean, ImGuiTabItemFlags_None)) {
        ImGui::Text("This is the objects selector tab");
        ImGui::EndTabItem();
    }

    // Object adder tab
    // -------------------------------------
    if (ImGui::BeginTabItem("Add Objects", &boolean, ImGuiTabItemFlags_None)) {
        ImGui::Text("Add objects to the scene here");
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(window_x - (window_x * 0.15f), 0.0f));
    ImGui::SetNextWindowSize(ImVec2(window_x * 0.15f, window_y * 0.3f));
    ImGui::Begin("Second menu");

    ImGui::End();
}

void App::processMouseMovement(float mouse_xpos, float mouse_ypos) {
    // Process mouse movement here
}

void App::processScreenResize(float new_window_x, float new_window_y) {
    window_x = static_cast<int>(new_window_x);
    window_y = static_cast<int>(new_window_y);

    renderer.processScreenResize(window_x, window_y);
}

ChunkPos App::getChunkPos(int x_pos, int y_pos, int z_pos) {
    ChunkPos return_chunk_pos;

    return_chunk_pos.x = (x_pos - (x_pos < 0 ? Chunk::CHUNK_SIZE - 1 : 0)) / Chunk::CHUNK_SIZE;
    return_chunk_pos.y = (y_pos - (y_pos < 0 ? Chunk::CHUNK_SIZE - 1 : 0)) / Chunk::CHUNK_SIZE;
    return_chunk_pos.z = (z_pos - (z_pos < 0 ? Chunk::CHUNK_SIZE - 1 : 0)) / Chunk::CHUNK_SIZE;

    return return_chunk_pos;
}

ChunkPos App::getChunkPos(glm::vec3 pos) {
    ChunkPos return_chunk_pos(
        std::floor(pos.x / (Chunk::CHUNK_SIZE * Block::side_length)),
        std::floor(pos.y / (Chunk::CHUNK_SIZE * Block::side_length)),
        std::floor(pos.z / (Chunk::CHUNK_SIZE * Block::side_length)));

    return return_chunk_pos;
}