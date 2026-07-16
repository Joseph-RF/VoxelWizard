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
        integrateGeneratedColumns();
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
    ColumnPos column_pos = chunk_pos.getColumnPos();
    if (!(column_pos == this->current_column_pos)) {
        this->current_column_pos = column_pos;
        //this->current_chunk_pos = ChunkPos(0, 0, 0);
        updateChunkQueues();
        updateChunksToBeRendered();
    }
}

void App::updateColumnEdgeOccupancy(Column& column) {

    static const std::array<std::pair<ChunkNeighbour, ChunkPos>, 6> column_neighbour_positions{ {
        {ChunkNeighbour::LEFT, ChunkPos{-1, 0, 0}},
        {ChunkNeighbour::RIGHT, ChunkPos{1, 0, 0}},
        {ChunkNeighbour::BEHIND, ChunkPos{0, 0, -1}},
        {ChunkNeighbour::IN_FRONT, ChunkPos{0, 0, 1}},
    } };

    // Iterate through each individual chunk in a chunk stack for every neighbour direction
    for (const auto& pair : column_neighbour_positions) {
        ColumnPos neighbour_column_pos{
                column.column_pos.x + pair.second.x,
                column.column_pos.z + pair.second.z
        };

        // Search in the chunk stack hash map
        if (chunks.count(neighbour_column_pos) > 0) {
            for (Chunk& chunk : column.chunks) {

                chunk.padOccupancy(chunks.at(neighbour_column_pos)[chunk.chunk_pos.y], pair.first);
                // If the neighbour already existed in data, update its occupancy as well
                chunks.at(neighbour_column_pos)[chunk.chunk_pos.y].padOccupancy(chunk, reverseChunkNeighbour(pair.first));

                // Need to update the neighbouring chunk's mesh now
                chunks.at(neighbour_column_pos)[chunk.chunk_pos.y].setDirty(true);
            }
            // Neighbouring chunk stack needs to have its mesh refreshed
            stale_columns_vertices_helper.push(neighbour_column_pos);
        }
        else if (columns_on_helper_thread.count(neighbour_column_pos) > 0) {
            for (Chunk& chunk : column.chunks) {
                chunk.padOccupancy(helper_created_columns[columns_on_helper_thread.at(neighbour_column_pos)][chunk.chunk_pos.y], pair.first);
            }
        }
    }

    // Go through the chunks in the chunk stack from the bottom up except for last chunk (nothing above it)
    for (int i = 0; i < Column::COLUMN_HEIGHT - 1; ++i) {
        column.chunks[i].padOccupancy(column.chunks[i + 1], ChunkNeighbour::ABOVE);
    }
    // Go through the chunks in the chunk stack from the top down except for the last chunk (nothing below it)
    for (int i = Column::COLUMN_HEIGHT - 1; i > 0; --i) {
        column.chunks[i].padOccupancy(column.chunks[i - 1], ChunkNeighbour::BELOW);
    }
}

void App::updateChunkQueues() {
    updateColumnCreationQueue();
    updateColumnMeshCreationQueues();
}

void App::updateColumnCreationQueue() {
    std::unordered_set<ColumnPos, ColumnPosHashFunc> column_list; // Chunk stacks that SHOULD exist

    // Refresh the current creation queue
    column_creation_queue_main = std::queue<ColumnPos>();

    for (int i = -chunk_distance; i <= chunk_distance; ++i) {
        for (int k = -chunk_distance; k <= chunk_distance; ++k) {
            ColumnPos column_pos = ColumnPos{ current_column_pos.x + i, current_column_pos.z + k };
            if (chunks.count(column_pos) == 0) {
                // If a chunk stack at column_pos doesn't exist, add it to the queue
                // to be created
                column_creation_queue_main.push(column_pos);
            }
            column_list.insert(column_pos);
        }
    }

    // Create deletion queue
    for (auto& element : chunks) {
        if (column_list.count(element.first) == 0) {
            // If cannot find column_pos of current chunk stack in column_list,
            // queue for deletion
            column_deletion_queue.push(element.first);
        }
    }
}

void App::updateColumnMeshCreationQueues() {
    // TODO: Will need to modify this when block edition is added.

    // Create list of chunk stacks whose chunk meshes need to be created
    std::unordered_set<ColumnPos, ColumnPosHashFunc> stack_mesh_list;

    // Refresh the current chunk vertex and mesh creation queues
    columns_vertex_creation_queue_main = std::queue<ColumnPos>();
    column_mesh_creation_queue = std::queue<ColumnPos>();

    for (int i = -mesh_distance; i <= mesh_distance; ++i) {
        for (int k = -mesh_distance; k <= mesh_distance; ++k) {
            ColumnPos column_pos = ColumnPos{ current_column_pos.x + i, current_column_pos.z + k };

            stack_mesh_list.insert(column_pos);

            if (chunks.count(column_pos) == 0) {
                std::cout << "Chunk stack missing when vertex creation queue was being updated" << std::endl;
                column_creation_queue_main.push(column_pos);
                columns_vertex_creation_queue_main.push(column_pos);
                column_mesh_creation_queue.push(column_pos);
                continue;
            }
            /*
            * NOTE NOTE: REMOVING THIS FOR NOW
            * 
            // If a chunk at chunk_pos doesn't have a generated mesh, add to the appropriate creation queues
            if (!chunks.at(chunk_pos).mesh_generated) {
                chunk_vertex_creation_queue_main.push(chunk_pos);
                chunk_mesh_creation_queue.push(chunk_pos);
            }
            */
        }
    }

    // Remove meshes from GPU memory that aren't in the new list of meshes
    for (auto& chunk : chunks) {
        if (stack_mesh_list.count(chunk.first) == 0) {
            column_mesh_deletion_queue.push(chunk.first);
        }
    }
}

void App::swapCreationQueues() {
    // Pass queue data from main thread queues to helper thread queues
    column_creation_queue_main.swap(column_creation_queue_helper);
    std::queue<ColumnPos>().swap(column_creation_queue_main); // Empty queue on main

    columns_vertex_creation_queue_main.swap(columns_vertex_creation_queue_helper);
    std::queue<ColumnPos>().swap(columns_vertex_creation_queue_main); // Empty queue on main
}

void App::processCreationQueues() {
    // Create chunks that were in the creation queue as well as the vertices and indices of 
    // chunks that are in the mesh creation queue

    refreshStaleColumnVertices();

    generateColumns();

    generateColumnVertices();
}

void App::refreshStaleColumnVertices() {
    // Update the stale meshes for chunk stacks
    while (!stale_columns_vertices_helper.empty()) {
        ColumnPos column_pos = stale_columns_vertices_helper.front();

        // Don't need to do a dirtied check since it is done in generateVertices()

        // Search in chunks hash map
        if (chunks.count(column_pos) > 0) {
            chunks.at(column_pos).generateVertices();
            stale_mesh_creation_queue.push(column_pos);
        }
        stale_columns_vertices_helper.pop();
    }
}

void App::generateColumns() {
    while (!column_creation_queue_helper.empty()) {
        // Check that the chunk stack does not already exist anywhere
        ColumnPos column_pos = column_creation_queue_helper.front();
        if (chunks.count(column_pos) == 0 && columns_on_helper_thread.count(column_pos) == 0) {
            helper_created_columns.emplace_back(column_creation_queue_helper.front());
            columns_on_helper_thread.emplace(column_creation_queue_helper.front(), helper_created_columns.size() - 1);
        }
        column_creation_queue_helper.pop();
    }
    // Update the occupancy arrays to reflect occupancy data of neighbouring chunks
    for (Column& column : helper_created_columns) {
        updateColumnEdgeOccupancy(column);
    }
}

void App::generateColumnVertices() {
    while (!columns_vertex_creation_queue_helper.empty()) {

        ColumnPos column_pos = columns_vertex_creation_queue_helper.front();

        // Don't need to do a dirtied check since it is done in generateVertices()

        // Search in chunk stack hash map
        if (chunks.count(column_pos) > 0) {
            // TODO NOTE: REMOVED THE MESH GENERATED CHECK FOR GENERATING VERTICES
            chunks.at(column_pos).generateVertices();
        }
        // Search in chunks still sitting in the helper thread array
        else if (columns_on_helper_thread.count(column_pos) > 0) {
            // TODO NOTE: REMOVED THE MESH GENERATED CHECK FOR GENERATING VERTICES
            helper_created_columns[columns_on_helper_thread.at(column_pos)].generateVertices();
        }
        columns_vertex_creation_queue_helper.pop();
    }
}

void App::integrateGeneratedColumns() {
    // Pass chunks constructed by the helper thread to the main chunk hash map
    // Reset the structures used by helper thread to hold generated chunks
    
    for (Column& column : helper_created_columns) {
        ColumnPos key = column.column_pos;
        chunks.try_emplace(key, std::move(column));
    }
    std::vector<Column>().swap(helper_created_columns);
    std::unordered_map<ColumnPos, int, ColumnPosHashFunc>().swap(columns_on_helper_thread);
}

void App::swapBuffersAndGenerateMeshes() {
    // Swap the back and front buffers containing newly made vertex and index data
    // Construct the chunk meshes using the new vertex and index data

    ColumnPos column_pos;

    while (!column_mesh_creation_queue.empty()) {
        column_pos = column_mesh_creation_queue.front();
        if (chunks.count(column_pos)) {
            chunks.at(column_pos).swapVertexBuffers();
            chunks.at(column_pos).generateMeshes();
        }
        column_mesh_creation_queue.pop();
    }

    // Now swap buffers and generate meshes for chunks that had stale vertices and meshes
    while (!stale_mesh_creation_queue.empty()) {
        if (chunks.count(stale_mesh_creation_queue.front()) > 0) {
            chunks.at(stale_mesh_creation_queue.front()).swapVertexBuffers();
            chunks.at(stale_mesh_creation_queue.front()).refreshMeshes();
        }
        stale_mesh_creation_queue.pop();
    }

}

void App::processDeletionQueues() {
    // Delete chunks and chunk meshes sent to the deletion queue
    // Check that the chunks exist before deleting them
    ColumnPos column_pos;

    while (!column_mesh_deletion_queue.empty()) {
        column_pos = column_mesh_deletion_queue.front();
        if (chunks.count(column_pos) > 0) {
            chunks.at(column_pos).destroyMeshes();
        }
        column_mesh_deletion_queue.pop();
    }

    while (!column_deletion_queue.empty()) {
        if (chunks.count(column_deletion_queue.front()) > 0) {
            chunks.erase(column_deletion_queue.front());
        }
        column_deletion_queue.pop();
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
    columns_to_be_rendered = std::vector<ColumnPos>(std::pow(2 * render_distance + 1, 2));
    int counter = 0;
    for (int i = -render_distance; i <= render_distance; i++) {
        for (int k = -render_distance; k <= render_distance; k++) {
            columns_to_be_rendered[counter] = ColumnPos(current_column_pos.x + i, current_column_pos.z + k);
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
    renderer.render(RenderContext{camera, chunks, columns_to_be_rendered});

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