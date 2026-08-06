#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <iostream>

#include <imgui_utility.hpp>
#include <iwindowmanager.hpp>

class GLFWWindowManager : public IWindowManager {
public:
    GLFWWindowManager(int window_width, int window_height,
                      std::shared_ptr<EventManager> event_manager);

    bool createWindow() override;
    void update() override;
    void renderToWindow() override;
    bool shouldWindowClose() override;

    void newImGuiFrame() override;
    void toggleMouse() override;
    float getTime() override;

private:
    int window_width;
    int window_height;
    bool is_mouse_free;

    GLFWwindow* window;

    std::shared_ptr<EventManager> event_manager;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double mouse_xpos, double mouse_ypos);
    static void scrollCallback(GLFWwindow* window, double x_offset, double y_offset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

Key glfwKeyToEnumKey(int GLFW_key);
MouseButton glfwButtonToMouseButton(int GLFW_button);
