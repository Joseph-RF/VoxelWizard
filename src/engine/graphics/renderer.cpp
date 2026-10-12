#include "renderer.hpp"

#include <glad/glad.h>

#include <iostream>

Renderer::Renderer(int window_width, int window_height)
    : window_width(window_width)
    , window_height(window_height) {
    // Initialise other variables
    far_plane = 10000.f;
    
    subsamples = 4;

    multisample_fbo  = 0;
    multisample_rbo  = 0;
    intermediate_fbo = 0;

    screen_vbo = 0;
    screen_vao = 0;

    multisample_texture = 0;
    screen_texture      = 0;

    // Rendering parameters
    shininess = 32.0f;
    show_normals = false;
    fill_polygons = true;

    dir_light.direction = glm::vec3(0.0, 1.0, 0.4);
    dir_light.colour = glm::vec3(1.0, 1.0, 1.0);
    dir_light.ambient = 0.1f;
    dir_light.diffuse = 0.8f;
    dir_light.specular = 1.0f;

    // Camera parameters
    ortho = true;
    zoom = 0.38f;
}

Renderer::~Renderer() {
    glDeleteFramebuffers(1, &multisample_fbo);
    glDeleteFramebuffers(1, &intermediate_fbo);
}

void Renderer::init(std::shared_ptr<ResourceManager> resource_manager) {
    m_resource_manager = resource_manager;

    initUBOs();
    initFramebuffers();
    initScreenQuad();

    // Set OpenGL flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
}

void Renderer::render(const RenderContext& render_context) {
    renderPrep(render_context.camera);

    // Set uniforms
    setUniforms(render_context);

    // Render scene
    renderScene(render_context);

    // Render the player
    renderActors();

    // Render to screen
    renderScreen();
}

void Renderer::initUBOs() {

    // Create the normals and colours that will go in the UBO
    glm::vec4 normals[6] = {
        { -1.0,  0.0,  0.0, 1.0},
        {  1.0,  0.0,  0.0, 1.0},
        {  0.0, -1.0,  0.0, 1.0},
        {  0.0,  1.0,  0.0, 1.0},
        {  0.0,  0.0, -1.0, 1.0},
        {  0.0,  0.0,  1.0, 1.0},
    };

    glm::vec4 colours[6] = {
         {1.0, 1.0, 1.0, 1.0},
         {0.15, 0.6, 0.0, 1.0},
         {0.4, 0.4, 0.4, 1.0},
         {0.8, 0.8, 0.8, 1.0},
         {0.72, 0.67, 0.52, 1.0},
         {0.1, 0.1, 0.8, 1.0},
    };

    unsigned int chunks_shader_id = m_resource_manager->getShader("chunks").ID;
    unsigned int player_shader_id = m_resource_manager->getShader("player").ID;

    unsigned int matrices_index_chunks = glGetUniformBlockIndex(chunks_shader_id, "Matrices");
    unsigned int block_data_index_chunks = glGetUniformBlockIndex(chunks_shader_id, "BlockData");
    unsigned int matrices_index_player = glGetUniformBlockIndex(player_shader_id, "Matrices");

    glUniformBlockBinding(chunks_shader_id, matrices_index_chunks, 0); // Will set Matrices binding to 0
    glUniformBlockBinding(chunks_shader_id, block_data_index_chunks, 1); // Will set BlockData binding to 0
    glUniformBlockBinding(player_shader_id, matrices_index_player, 0); // Will set Matrices binding to 0

    // Create uniform buffer objects
    glGenBuffers(1, &matrices_ubo);
    glGenBuffers(1, &block_data_ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, block_data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(normals) + sizeof(colours), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, block_data_ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, block_data_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(normals), normals);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(normals), sizeof(colours), colours);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void Renderer::initFramebuffers() {
    glGenFramebuffers(1, &multisample_fbo);
    glGenTextures(1, &multisample_texture);
    glGenRenderbuffers(1, &multisample_rbo);

    // Colour attachment
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisample_texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, subsamples, GL_RGB, window_width,
                            window_height, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Stencil and buffer attachment
    glBindRenderbuffer(GL_RENDERBUFFER, multisample_rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, subsamples, GL_DEPTH24_STENCIL8, window_width,
                                     window_height);

    glBindFramebuffer(GL_FRAMEBUFFER, multisample_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           multisample_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              multisample_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Multisample Framebuffer is not complete!" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Now for the intermediate framebuffer and the screen quad texture
    glGenFramebuffers(1, &intermediate_fbo);
    glGenTextures(1, &screen_texture);

    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, intermediate_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Intermediate Framebuffer is not complete!" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initScreenQuad() {
    float vertices[] = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
                        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    glGenBuffers(1, &screen_vbo);
    glGenVertexArrays(1, &screen_vao);

    glBindVertexArray(screen_vao);
    glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Renderer::setSystems(std::shared_ptr<ActorRenderSystem> ars) {
    m_actor_render_system = ars;
}

void Renderer::processScreenResize(int new_window_width, int new_window_height) {
    window_width  = new_window_width;
    window_height = new_window_height;

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisample_texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, subsamples, GL_RGB, window_width,
                            window_height, GL_TRUE);
    glBindRenderbuffer(GL_RENDERBUFFER, multisample_rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, subsamples, GL_DEPTH24_STENCIL8, window_width,
                                     window_height);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Now for the intermediate framebuffer and the screen quad texture
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::renderPrep(Camera& camera) {
    // Render to multisample framebuffer to write to multisample texture
    glBindFramebuffer(GL_FRAMEBUFFER, multisample_fbo);
    glClearColor(0.38f, 0.77f, 0.94f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::setUniforms(const RenderContext& render_context) {
    // Set uniforms that are consistent between vertices
    Shader& chunk_shader = m_resource_manager->getShader("chunks");
    Shader& player_shader = m_resource_manager->getShader("player");
    // View and projection matrices won't change between objects
    glm::mat4 view = render_context.camera.lookAt();
    glm::mat4 projection = glm::perspective(
        glm::radians(render_context.camera.fov),
        (static_cast<float>(window_width) / static_cast<float>(window_height)), 0.1f, far_plane);

    if (ortho) {
        projection = glm::ortho(-(float)window_width * zoom / 2.f, 
                                 (float)window_width * zoom / 2.f, 
                                -(float)window_height * zoom / 2.f, 
                                 (float)window_height * zoom / 2.f, 
                                  0.1f,
                                  far_plane
        );
    }

    glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Vertex shader uniforms
    chunk_shader.use();
    chunk_shader.setFloat("side_length", Voxel::BLOCK_LENGTH);

    // Fragment shader uniforms for chunk shader
    chunk_shader.setVec3("viewer_pos", render_context.camera.pos);
    chunk_shader.setFloat("shininess", shininess);
    chunk_shader.setBool("show_normals", show_normals);
    chunk_shader.setVec3("dir_light.direction", dir_light.direction);
    chunk_shader.setVec3("dir_light.colour", dir_light.colour);
    chunk_shader.setFloat("dir_light.ambient", dir_light.ambient);
    chunk_shader.setFloat("dir_light.diffuse", dir_light.diffuse);
    chunk_shader.setFloat("dir_light.specular", dir_light.specular);

    player_shader.use();
    player_shader.setVec3("viewer_pos", render_context.camera.pos);
    player_shader.setFloat("shininess", shininess);
    player_shader.setVec3("dir_light.direction", dir_light.direction);
    player_shader.setVec3("dir_light.colour", dir_light.colour);
    player_shader.setFloat("dir_light.ambient", dir_light.ambient);
    player_shader.setFloat("dir_light.diffuse", dir_light.diffuse);
    player_shader.setFloat("dir_light.specular", dir_light.specular);
}

void Renderer::renderScene(const RenderContext& render_context) {

    Shader& chunk_shader = m_resource_manager->getShader("chunks");

    glBindFramebuffer(GL_FRAMEBUFFER, multisample_fbo);

    if (!fill_polygons) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    // Call object draw functions
    std::vector<Column>& columns = render_context.world.getColumns();
    for (Column& column : columns) {
        column.draw(chunk_shader);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::renderActors() {

    // TODO:
    // Use the actor render system to render actors
    // Probably just a simple actor render system call
    m_actor_render_system->render();
}

void Renderer::renderScreen() {
    // Blit multisample framebuffer to intermediate framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, multisample_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediate_fbo);
    glBlitFramebuffer(0, 0, window_width, window_height, 0, 0, window_width, window_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(screen_vao);
    m_resource_manager->getShader("screen").use();
    m_resource_manager->getShader("screen").setInt("screenTexture", 0);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
