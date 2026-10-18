#include "app.hpp"

#include "../engine/ecs/coordinator.hpp"
#include "components.hpp"
#include "actor_render_system.hpp"

#include <chrono>

Coordinator g_coordinator;

App::App(int window_x, int window_y)
    : renderer(window_x, window_y), 
      camera(Voxel::WORLD_SIZE, Voxel::CHUNK_SIZE, Voxel::BLOCK_LENGTH, window_x, window_y) {
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

    // Resource manager
    resource_manager = std::make_shared<ResourceManager>();
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

    initResources();

    renderer.init(resource_manager);

    initCoordinator();
    initSystems();
    initWorld();
    initEntities();

    return true;
}

void App::initCoordinator() {
    g_coordinator.init();
    // Register the components
    g_coordinator.registerComponent<Transform>();
    g_coordinator.registerComponent<Renderable>();
    g_coordinator.registerComponent<Controllable>();
}

void App::initSystems() {
    std::shared_ptr<ActorRenderSystem> actor_render_system = g_coordinator.registerSystem<ActorRenderSystem>();
    {
        Signature signature;
        signature.set(g_coordinator.getComponentType<Transform>());
        signature.set(g_coordinator.getComponentType<Renderable>());
        g_coordinator.setSystemSignature<ActorRenderSystem>(signature);
    }
    actor_render_system->init(resource_manager);
    renderer.setSystems(actor_render_system);

    actor_control_system = g_coordinator.registerSystem<ActorControlSystem>();
    {
        Signature signature;
        signature.set(g_coordinator.getComponentType<Transform>());
        signature.set(g_coordinator.getComponentType<Controllable>());
        g_coordinator.setSystemSignature<ActorControlSystem>(signature);
    }
    actor_control_system->init();
    // Set the rest of the systems
}

void App::initResources()
{
    // Shaders
    // ------------------------------------------
    resource_manager->createShader("chunks", SHADERS_PATH "chunk_vshader.glsl", SHADERS_PATH "chunk_fshader.glsl", "");
    resource_manager->createShader("player", SHADERS_PATH "player_vshader.glsl", SHADERS_PATH "player_fshader.glsl", "");
    resource_manager->createShader("screen", SHADERS_PATH "screen_vshader.glsl",
                      SHADERS_PATH "screen_fshader.glsl", "");

    // Meshes
    // ------------------------------------------
    resource_manager->createMeshFromObj("player", MODELS_PATH "wizardplaceholder.obj");
}

void App::initWorld() {
    world.init();
}

void App::initEntities() {

    Entity player;
    player = g_coordinator.createEntity();

    g_coordinator.addComponent<Transform>(player, 
        Transform{
            {0.0f, 10.f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            {1.0f, 1.0f, 1.0f}
        }
    );

    g_coordinator.addComponent<Renderable>(player,
        Renderable{
            resource_manager->getMeshID("player"),
            resource_manager->getShaderID("player")
        }
    );

    g_coordinator.addComponent<Controllable>(player, Controllable{250.0f});

    // Set the camera's target
    camera.setTargetEntity(player);
}

void App::update() {
    float current_frame = window_manager->getTime();
    delta_time          = current_frame - last_frame;
    last_frame          = current_frame;

    processEvents();

    actor_control_system->update(delta_time, camera.front - camera.pos, camera.world_space_up);

    // Run the physics here

    camera.update();

    window_manager->update();

    event_manager->update();
}

void App::processEvents() {
    for (; !event_manager->events.empty(); event_manager->events.pop()) {
        // Consider using a switch statement for the below
        switch (event_manager->events.front().action) {
        case Action::SCREEN_RESIZE:
            processScreenResize(event_manager->events.front().mouse_xpos,
                                event_manager->events.front().mouse_ypos);
            break;
        case Action::MOVE_FORWARD:
            //camera.processKeyboard(FORWARD, delta_time);
            actor_control_system->push_action(Action::MOVE_FORWARD);
            break;
        case Action::MOVE_LEFT:
            //camera.processKeyboard(LEFT, delta_time);
            actor_control_system->push_action(Action::MOVE_LEFT);
            break;
        case Action::MOVE_BACKWARD:
            //camera.processKeyboard(BACKWARD, delta_time);
            actor_control_system->push_action(Action::MOVE_BACKWARD);
            break;
        case Action::MOVE_RIGHT:
            //camera.processKeyboard(RIGHT, delta_time);
            actor_control_system->push_action(Action::MOVE_RIGHT);
            break;
        case Action::MOVE_UP:
            //camera.processKeyboard(UP, delta_time);
            break;
        case Action::MOVE_DOWN:
            //camera.processKeyboard(DOWN, delta_time);
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
    renderer.render(RenderContext{camera, world});

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
        std::string fps_text = "FPS: " + std::to_string((int)std::round(1.f / delta_time));
        ImGui::Text(fps_text.c_str());

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
        ImGui::Checkbox("Perspective or Orthographic", &camera.ortho);

        ImGui::SetNextItemWidth(120.f);
        ImGui::SliderFloat("Zoom", &camera.zoom, 0.01f, 1.f);

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

void App::initSceneTEST() {
    constexpr int iterations = 1;

    std::array<std::chrono::milliseconds, iterations> totals;

    for (unsigned int i = 0; i < iterations; ++i) {
        world = World();

        auto start = std::chrono::high_resolution_clock::now();

        initWorld();

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
