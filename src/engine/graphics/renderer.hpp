#pragma once

#include "camera.hpp"
#include "shader.hpp"
#include "game/world/world_objects.hpp"
#include "engine/core/resource_manager.hpp"
#include "game/world/world.hpp"
#include "game/actor_render_system.hpp"

#include <memory>
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
    World& world;
};

class Renderer {
public:
    Renderer(int window_width, int window_height);
    ~Renderer();

    void init(std::shared_ptr<ResourceManager> resource_manager);

    void setSystems(std::shared_ptr<ActorRenderSystem> ars);

    void render(const RenderContext& render_context);

    void processScreenResize(int new_window_width, int new_window_height);

    // Rendering parameters
    float shininess;
    bool show_normals;
    bool fill_polygons;
    DirLight dir_light;

    // Camera parameters
    bool ortho;
    float zoom;

private:
    void initUBOs();
    void initFramebuffers();
    void initScreenQuad();

    void renderPrep(Camera& camera);
    void setUniforms(const RenderContext& render_context);
    void renderScene(const RenderContext& render_context);
    void renderActors();
    void renderScreen();

    // Window properties
    int window_width;
    int window_height;

    // Projection properties
    float far_plane;

    // Resource Manager
    std::shared_ptr<ResourceManager> m_resource_manager;

    // Actor render system
    std::shared_ptr<ActorRenderSystem> m_actor_render_system;

    // Subsamples
    unsigned int subsamples;

    // Buffer objects
    unsigned int multisample_fbo;
    unsigned int multisample_rbo;
    unsigned int intermediate_fbo;
    unsigned int matrices_ubo;
    unsigned int block_data_ubo;

    // Screen quad buffers
    unsigned int screen_vbo;
    unsigned int screen_vao;

    // Textures
    unsigned int multisample_texture;
    unsigned int screen_texture;
};