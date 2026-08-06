#include <glfwwindowmanager.hpp>

GLFWWindowManager::GLFWWindowManager(int window_width, int window_height,
                                     std::shared_ptr<EventManager> event_manager)
    : window_width(window_width)
    , window_height(window_height)
    , is_mouse_free(false) {
    window              = nullptr;
    this->event_manager = event_manager;
}

bool GLFWWindowManager::createWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); Needed for Mac OS X

    // Create GLFW window object to old window data
    window = glfwCreateWindow(window_width, window_height, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Set the input handler as the GLFW user pointer
    glfwSetWindowUserPointer(window, this);

    // Callback function for framebuffer resizing
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Callback functions involving the mouse
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Set a callback function for keyboard input
    glfwSetKeyCallback(window, keyCallback);

    // OpenGL
    // ----------------------------------------------------
    // Initialialise GLAD in order to get access to OpenGL function pointers before
    // using OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    // Let OpenGL know the size of the viewport (can make it smaller if we want
    // to display other things in the GLFW window
    glViewport(0, 0, window_width,
               window_height); // x-pos of lower left corner, y-pos, width, height

    // ImGUI
    // ----------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Set ImGui style using utility function
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui_Utility::setStyle(style);

    // Lock the mouse in place into the middle of the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return true;
}

void GLFWWindowManager::update() {
    glfwPollEvents();
}

void GLFWWindowManager::renderToWindow() {
    // Swap the front and back buffer (look up Double Buffer)
    glfwSwapBuffers(window);
}

bool GLFWWindowManager::shouldWindowClose() {
    return glfwWindowShouldClose(window);
}

void GLFWWindowManager::newImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GLFWWindowManager::toggleMouse() {
    if (is_mouse_free) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        is_mouse_free = false;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        is_mouse_free = true;
    }
}

float GLFWWindowManager::getTime() {
    return static_cast<float>(glfwGetTime());
}

void GLFWWindowManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    GLFWWindowManager* manager = static_cast<GLFWWindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->event_manager) {
        glViewport(0, 0, width, height);
        manager->event_manager->addScreenResizeEvent(width, height);
    }
}

void GLFWWindowManager::mouseCallback(GLFWwindow* window, double mouse_xpos, double mouse_ypos) {
    GLFWWindowManager* manager = static_cast<GLFWWindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->event_manager) {
        bool turning = !manager->is_mouse_free;
        // Call the input function here
        manager->event_manager->addMouseMovedEvent(mouse_xpos, mouse_ypos, turning);
    }
}

void GLFWWindowManager::scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
    GLFWWindowManager* manager = static_cast<GLFWWindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->event_manager) {
        // Call the input function here
        float scroll_x = static_cast<float>(x_offset);
        float scroll_y = static_cast<float>(y_offset);

        manager->event_manager->addScrollEvent(scroll_x, scroll_y);
    }
}

void GLFWWindowManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    GLFWWindowManager* manager = static_cast<GLFWWindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->event_manager) {
        // Call the input function here
        bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        manager->event_manager->addMouseClickEvent(glfwButtonToMouseButton(button), pressed);
    }
}

void GLFWWindowManager::keyCallback(GLFWwindow* window, int key, int scancode, int action,
                                    int mods) {
    GLFWWindowManager* manager = static_cast<GLFWWindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->event_manager) {
        bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

        if (pressed && key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
            return;
        }

        manager->event_manager->keyEvent(glfwKeyToEnumKey(key), pressed);
    }
}

std::unique_ptr<IWindowManager> makeGLFWWindowManager(int window_width, int window_height,
                                                      std::shared_ptr<EventManager> event_manager) {
    return std::make_unique<GLFWWindowManager>(window_width, window_height, event_manager);
}

Key glfwKeyToEnumKey(int GLFW_key) {
    switch (GLFW_key) {
    case GLFW_KEY_W:
        return Key::W;
    case GLFW_KEY_A:
        return Key::A;
    case GLFW_KEY_S:
        return Key::S;
    case GLFW_KEY_D:
        return Key::D;
    case GLFW_KEY_LEFT_CONTROL:
        return Key::L_CTRL;
    case GLFW_KEY_SPACE:
        return Key::SPACE;
    case GLFW_KEY_M:
        return Key::M;
    default:
        return Key::OTHER;
    }
}

MouseButton glfwButtonToMouseButton(int GLFW_button) {
    switch (GLFW_button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        return MouseButton::LEFT;
    case GLFW_MOUSE_BUTTON_RIGHT:
        return MouseButton::RIGHT;
    default:
        return MouseButton::OTHER;
    }
}
