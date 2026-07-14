#pragma once

#include <camera.hpp>
#include <memory>
#include <shader.hpp>
#include "game_objects.hpp"

#include <vector>

struct RenderContext {
    // Throw objects to be rendered in here
    Camera& camera;
    std::unordered_map<ChunkPos, Chunk, HashFunction>& chunks;
    std::vector<ChunkPos>& chunks_to_be_rendered;
};

class Renderer {
public:
    Renderer(int window_width, int window_height);
    ~Renderer();

    void init();

    void render(const RenderContext& render_context);

    void processScreenResize(int new_window_width, int new_window_height);

private:
    void initShaders();
    void initFramebuffers();
    void initScreenQuad();

    void renderPrep(Camera& camera);
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

    // Mesh
    std::unordered_map<unsigned int, std::vector<float>> meshes;
};
