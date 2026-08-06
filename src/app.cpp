#include "app.hpp"

#include <chrono>

App::App(int window_x, int window_y)
    : renderer(window_x, window_y), camera(WORLD_SIZE, Chunk::CHUNK_SIZE, Block::side_length) {
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
    // Set noise generator parameters
    Column::configureNoiseGenerator();

    for (int i = -HALF_WORLD_SIZE; i < HALF_WORLD_SIZE; ++i) {
        for (int k = -HALF_WORLD_SIZE; k < HALF_WORLD_SIZE; ++k) {
            columns.emplace_back(ColumnPos(i, k));
        }
    }

    // Update column occupancy data now that they've all been created
    for (Column& column : columns) {
        updateColumnEdgeOccupancy(column);
    }

    // Now that chunk data is ready, generate vertices and meshes
    for (Column& column : columns) {
        column.generateVertices();
        column.generateMeshes();
    }
}

void App::update() {
    float current_frame = window_manager->getTime();
    delta_time          = current_frame - last_frame;
    last_frame          = current_frame;

    runActions();

    window_manager->update();

    event_manager->update();
}

void App::updateColumnEdgeOccupancy(Column& column) {

    static const std::array<std::pair<ChunkNeighbour, ChunkPos>, 4> column_neighbour_positions{ {
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

        // Check the neighbour is in bounds a.k.a exists
        if (!inBounds(neighbour_column_pos)) { continue; }

        for (Chunk& chunk : column.chunks) {
            chunk.padOccupancy(columns[getColumnIndex(neighbour_column_pos)][chunk.chunk_pos.y], pair.first);
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
    renderer.render(RenderContext{camera, columns});

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
    if (ImGui::BeginTabItem("Tweak Rendering Parameters", &boolean, ImGuiTabItemFlags_None)) {
        ImGui::Text("Camera position");
        
        ImGui::InputFloat("x pos", &camera.pos.x, 0.25f, 1.0f,
                  "%.2f");
        ImGui::InputFloat("y pos", &camera.pos.y, 0.25f, 1.0f,
          "%.2f");
        ImGui::InputFloat("z pos", &camera.pos.z, 0.25f, 1.0f,
          "%.2f");

        ImGui::NewLine();

        ImGui::Text("Light direction");

        ImGui::SetNextItemWidth(120.f);
        ImGui::SliderFloat("x component", &renderer.dir_light.direction.x, -1.0f, 1.0f);
        ImGui::SetNextItemWidth(120.f);
        ImGui::SliderFloat("y component", &renderer.dir_light.direction.y, -1.0f, 1.0f);
        ImGui::SetNextItemWidth(120.f);
        ImGui::SliderFloat("z component", &renderer.dir_light.direction.z, -1.0f, 1.0f);

        ImGui::Separator();
        ImGui::Separator();

        ImGui::ColorEdit3("Light Colour", glm::value_ptr(renderer.dir_light.colour));

        ImGui::Separator();
        ImGui::Separator();

        ImGui::SetNextItemWidth(120.f);
        ImGui::SliderFloat("Shininess", &renderer.shininess, 0.0f, 200.0f);

        ImGui::Text("Toggle Normal Visualisation");
        ImGui::Checkbox("Visualise Normals", &renderer.show_normals);

        ImGui::Text("Toggle Fill Polygons");
        ImGui::Checkbox("Fill Polygons", &renderer.fill_polygons);

        ImGui::Text("Toggle Camera Projection");
        ImGui::Checkbox("Perspective or Orthographic", &renderer.ortho);

        ImGui::SetNextItemWidth(120.f);
        ImGui::SliderFloat("Zoom", &renderer.zoom, 0.01f, 1.f);

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
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

inline int App::getColumnIndex(ColumnPos column_pos) {

    int x = column_pos.x + HALF_WORLD_SIZE;
    int z = column_pos.z + HALF_WORLD_SIZE;

    int index = (column_pos.z + HALF_WORLD_SIZE) + WORLD_SIZE * (column_pos.x + HALF_WORLD_SIZE);

    if (index > WORLD_SIZE_SQ - 1 || index < 0) {
        std::cout << "Column Index is out of bounds" << std::endl;
        return 0;
    }

    return index;
}

inline bool App::inBounds(ColumnPos column_pos) {
    if (column_pos.x > HALF_WORLD_SIZE - 1 || column_pos.x < -HALF_WORLD_SIZE) {
        return false;
    }
    if (column_pos.z > HALF_WORLD_SIZE - 1 || column_pos.z < -HALF_WORLD_SIZE) {
        return false;
    }
    return true;
}

void App::initSceneTEST() {
    constexpr int iterations = 1;

    std::array<std::chrono::milliseconds, iterations> totals;

    for (unsigned int i = 0; i < iterations; ++i) {
        std::vector<Column>().swap(columns);

        auto start = std::chrono::high_resolution_clock::now();

        initScene();

        // Record ending time
        auto stop = std::chrono::high_resolution_clock::now();

        totals[i] = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    }

    std::chrono::milliseconds mean = std::chrono::milliseconds(0);

    for (auto total : totals) {
        mean += total;
        mean /= iterations;
    }

    std::cout << "Average Time taken: " << mean.count() << " milliseconds" << std::endl;
}
