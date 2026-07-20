#pragma once

#include <camera.hpp>
#include <memory>
#include <shader.hpp>
#include "game_objects.hpp"

#include <vector>

struct DirLight {
    glm::vec3 direction;

    glm::vec3 colour;

    float ambient;
    float diffuse;
    float specular;
};

struct RenderContext {
    // Throw objects to be rendered in here
    Camera& camera;
    std::unordered_map<ColumnPos, Column, ColumnPosHashFunc>& chunks;
    std::vector<ColumnPos>& columns_to_be_rendered;

};

class Renderer {
public:
    Renderer(int window_width, int window_height);
    ~Renderer();

    void init();

    void render(const RenderContext& render_context);

    void processScreenResize(int new_window_width, int new_window_height);

    // Rendering parameters
    float shininess;
    bool show_normals;
    bool fill_polygons;
    DirLight dir_light;

private:
    void initShaders();
    void initFramebuffers();
    void initScreenQuad();

    void renderPrep(Camera& camera);
    void setUniforms(const RenderContext& render_context);
    void renderScene(const RenderContext& render_context);
    void renderScreen();

    // Window properties
    int window_width;
    int window_height;

    // Projection properties
    float far_plane;

    // Shaders
    ShaderLibrary shader_lib;

    // Subsamples
    unsigned int subsamples;

    // Buffer objects
    unsigned int multisample_fbo;
    unsigned int multisample_rbo;
    unsigned int intermediate_fbo;

    // Screen quad buffers
    unsigned int screen_vbo;
    unsigned int screen_vao;

    // Textures
    unsigned int multisample_texture;
    unsigned int screen_texture;
};