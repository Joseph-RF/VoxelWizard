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

    // Camera
    active_camera = &engine_camera;
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

    // Initialise renderer
    renderer.init();

    // Initialise static variables
    initObjects();

    // Initialise the scene
    initScene();

    return true;
}

void App::initObjects() {

}

void App::initScene() {
    createChunks();
    updateChunksToBeRendered();
}

void App::update() {
    float current_frame = window_manager->getTime();
    delta_time          = current_frame - last_frame;
    last_frame          = current_frame;

    runActions();

    checkCurrentChunk();

    window_manager->update();

    event_manager->update();
}

void App::checkCurrentChunk() {
    ChunkPos chunk_pos = getChunkPos(active_camera->pos);
    if (!(chunk_pos == this->current_chunk_pos)) {
        this->current_chunk_pos = chunk_pos;
        updateChunkList();
        updateMeshList();
        updateChunksToBeRendered();
    }
}

void App::updateChunks() {
    // Fill this in when chunk edition is added
}

void App::updateChunksToBeRendered() {
    chunks_to_be_rendered = std::vector<ChunkPos>(std::pow(2 * render_distance + 1, 2));
    int counter = 0;
    for (int i = -render_distance; i <= render_distance; i++) {
        for (int k = -render_distance; k <= render_distance; k++) {
            chunks_to_be_rendered[counter] = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);
            counter++;
        }
    }
}

void App::updateChunkList() {
    // Create list of chunks to be kept in memory
    std::unordered_set<ChunkPos, HashFunction> chunk_list;
    std::queue<ChunkPos> chunk_deletion_queue;

    for (int i = -chunk_distance; i <= chunk_distance; ++i) {
        for (int k = -chunk_distance; k <= chunk_distance; ++k) {
            ChunkPos chunk_pos = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);
            if (chunks.count(chunk_pos) == 0) {
                // If a chunk at chunk_pos doesn't exist in memory, create it
                chunks.insert_or_assign(chunk_pos, Chunk(chunk_pos));
            }
            chunk_list.insert(chunk_pos);
        }
    }

    // Create deletion queue & update the activity/visibility of blocks in all chunks
    for (auto& element : chunks) {

        updateChunkBlockActivity(element.first);

        if (chunk_list.count(element.first) == 0) {
            // If cannot find chunk_pos of current chunk in chunk_list, delete it
            chunk_deletion_queue.push(element.first);
        }
    }

    // Delete chunks found in the deletion queue
    while (!chunk_deletion_queue.empty()) {
        chunks.erase(chunk_deletion_queue.front());
        chunk_deletion_queue.pop();
    }
}

void App::updateMeshList() {
    // Create list of chunks whose meshes need to be created
    std::unordered_set<ChunkPos, HashFunction> mesh_list;
    for (int i = -mesh_distance; i <= mesh_distance; ++i) {
        for (int k = -mesh_distance; k <= mesh_distance; ++k) {
            ChunkPos chunk_pos = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);

            if (chunks.count(chunk_pos) == 0) {
                std::cout<<"Chunk missing when mesh list was being updated"<<std::endl;
                chunks.insert_or_assign(chunk_pos, Chunk(chunk_pos));
            }
            // If a chunk at chunk_pos doesn't have a generated mesh, generate it
            if (!chunks.at(chunk_pos).mesh_generated) {
                chunks.at(chunk_pos).generateMesh();
            }
            mesh_list.insert(chunk_pos);
        }
    }

    // Remove meshes from memory if they aren't in the new list of meshes
    for (auto& element : chunks) {
        if (mesh_list.count(element.first) == 0) {
            // If cannot find chunk_pos of current chunk in chunk_list, remove its mesh from memory
            chunks.at(element.first).destroyMesh();
        }
    }
}

void App::updateChunkBlockActivity(ChunkPos chunk_pos) {
    // Go through the chunks blocks and see if any blocks need to have their activity updated
    for (int i = 0; i < Chunk::CHUNK_SIZE; ++i) {
        for (int j = 0; j < Chunk::CHUNK_SIZE; ++j) {
            for (int k = 0; k < Chunk::CHUNK_SIZE; ++k) {
                updateBlockActivity(chunks[chunk_pos].blocks[i][j][k], Chunk::CHUNK_SIZE * chunk_pos.x + i,
                    Chunk::CHUNK_SIZE * chunk_pos.y + j, Chunk::CHUNK_SIZE * chunk_pos.z + k);
            }
        }
    }
}

void App::updateBlockActivity(Block& block, int x_pos, int y_pos, int z_pos) {

    if (!isBlockOpaque(x_pos - 1, y_pos, z_pos)) {
        block.setActive(true);
        return;
    }
    if (!isBlockOpaque(x_pos + 1, y_pos, z_pos)) {
        block.setActive(true);
        return;
    }
    if (!isBlockOpaque(x_pos, y_pos - 1, z_pos)) {
        block.setActive(true);
        return;
    }
    if (!isBlockOpaque(x_pos, y_pos + 1, z_pos)) {
        block.setActive(true);
        return;
    }
    if (!isBlockOpaque(x_pos, y_pos, z_pos - 1)) {
        block.setActive(true);
        return;
    }
    if (!isBlockOpaque(x_pos, y_pos, z_pos + 1)) {
        block.setActive(true);
        return;
    }

    block.setActive(false);
}

bool App::isBlockOpaque(int x_pos, int y_pos, int z_pos) {

    // TODO: Make sure the logic for finding the x, y and z index are correct

    int x = x_pos % Chunk::CHUNK_SIZE;
    int y = y_pos % Chunk::CHUNK_SIZE;
    int z = z_pos % Chunk::CHUNK_SIZE;

    if (x_pos < 0) {
        x = 16 + x;
        x = x % Chunk::CHUNK_SIZE;
    }
    if (y_pos < 0) {
        y = 16 + y;
        y = y % Chunk::CHUNK_SIZE;
    }
    if (z_pos < 0) {
        z = 16 + z;
        z = z % Chunk::CHUNK_SIZE;
    }

    ChunkPos chunk_pos = getChunkPos(x_pos, y_pos, z_pos);

    if (chunks.count(chunk_pos) < 1) {
        return false;
    }

    if (chunks[chunk_pos].blocks[x][y][z].getBlockType() == BlockType::Air) {
        return false;
    }
    return true;
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
            active_camera->processKeyboard(FORWARD, delta_time);
            break;
        case Action::MOVE_LEFT:
            active_camera->processKeyboard(LEFT, delta_time);
            break;
        case Action::MOVE_BACKWARD:
            active_camera->processKeyboard(BACKWARD, delta_time);
            break;
        case Action::MOVE_RIGHT:
            active_camera->processKeyboard(RIGHT, delta_time);
            break;
        case Action::MOVE_UP:
            active_camera->processKeyboard(UP, delta_time);
            break;
        case Action::MOVE_DOWN:
            active_camera->processKeyboard(DOWN, delta_time);
            break;
        case Action::TURN: {
            float mouse_xoffset = event_manager->events.front().mouse_xpos - last_mouse_xpos;
            float mouse_yoffset = event_manager->events.front().mouse_ypos - last_mouse_ypos;
            last_mouse_xpos += mouse_xoffset;
            last_mouse_ypos += mouse_yoffset;

            if (!first_mouse_movement) {
                active_camera->processMouse(mouse_xoffset, mouse_yoffset);
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
            active_camera->processScroll(event_manager->events.front().scroll_x,
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
    renderer.render(RenderContext{active_camera, chunks, chunks_to_be_rendered});

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
        ImGui::InputFloat("x pos", &active_camera->pos.x, 0.25f, 1.0f,
                  "%.2f");
        ImGui::InputFloat("y pos", &active_camera->pos.y, 0.25f, 1.0f,
          "%.2f");
        ImGui::InputFloat("z pos", &active_camera->pos.z, 0.25f, 1.0f,
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

void App::createChunks() {
    // TODO: What if we only want to create SOME chunks. This way need to redo the whole thing
    // For now will just do one layer of chunks (constant y)
    for (int i = -render_distance; i <= render_distance; ++i) {
        for (int k = -render_distance; k <= render_distance; ++k) {
            ChunkPos chunk_pos = ChunkPos(current_chunk_pos.x + i, 0, current_chunk_pos.z + k);
            if (chunks.count(chunk_pos) == 0) {
                chunks.insert_or_assign(chunk_pos, Chunk(chunk_pos));
            }
        }
    }
    // Once all chunks have been created, update block visibilities and generate meshes
    for (auto& element : chunks) {
        updateChunkBlockActivity(element.first);
        element.second.generateMesh();
    }
}

ChunkPos App::getChunkPos(int x_pos, int y_pos, int z_pos) {
    ChunkPos return_chunk_pos{
        x_pos / Chunk::CHUNK_SIZE,
        y_pos / Chunk::CHUNK_SIZE,
        z_pos / Chunk::CHUNK_SIZE
    };

    if (x_pos < 0) {return_chunk_pos.x--;}
    if (y_pos < 0) {return_chunk_pos.y--;}
    if (z_pos < 0) {return_chunk_pos.z--;}

    return return_chunk_pos;
}

ChunkPos App::getChunkPos(glm::vec3 pos) {
    ChunkPos return_chunk_pos(
        std::floor(pos.x / float(Chunk::CHUNK_SIZE)),
        std::floor(pos.y / float(Chunk::CHUNK_SIZE)),
        std::floor(pos.z / float(Chunk::CHUNK_SIZE)));

    return return_chunk_pos;
}
