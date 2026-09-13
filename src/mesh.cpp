#include <glad/glad.h>

#include <mesh.hpp>

Mesh::Mesh(const std::vector<VertexData>& vertices) {

    mesh_mode = MeshMode::NON_INDEXED;

    static constexpr float quad_vertices[12] = {
        // positions (x, y, z) for quad facing +y centred at origin (0, 0, 0)
        -0.5f, 0.f, -0.5f,
        -0.5f, 0.f,  0.5f,
         0.5f, 0.f, -0.5f,
         0.5f, 0.f,  0.5f,
    };

    // Configure instance array buffer
    glGenBuffers(1, &instance_array_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instance_array_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Configure standard buffer and vertex array
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set instance array data
    glBindBuffer(GL_ARRAY_BUFFER, instance_array_vbo);
    // Will use AttribIPointer to keep the data in GLSL as an int and not cast to float
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(VertexData), (void*)(offsetof(VertexData, packed_vertex_data)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

Mesh::Mesh(const std::vector<VertexData>& vertices, const std::vector<unsigned int>& indices) {

    mesh_mode = MeshMode::INDEXED;

    
}

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {

    mesh_mode = MeshMode::INDEXED;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    triangle_count = indices.size() / 3;
}

Mesh::Mesh(Mesh&& mesh) noexcept {

    // Moving buffer data from mesh to this.
    this->vbo = mesh.vbo;
    this->instance_array_vbo = mesh.instance_array_vbo;
    this->ebo = mesh.ebo;
    this->vao = mesh.vao;

    this->vertices_count = mesh.vertices_count;
    this->triangle_count = mesh.triangle_count;

    this->mesh_mode = mesh.mesh_mode;

    mesh.vbo = 0;
    mesh.instance_array_vbo = 0;
    mesh.ebo = 0;
    mesh.vao = 0;

    mesh.vertices_count = 0;
    mesh.triangle_count = 0;
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept {
    if (this == &mesh) {
        return *this;
    }

    // Have already been initialised so need delete existing buffers and vertex arrays (assuming they exist)
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &instance_array_vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    this->vbo = mesh.vbo;
    this->instance_array_vbo = mesh.instance_array_vbo;
    this->ebo = mesh.ebo;
    this->vao = mesh.vao;

    this->vertices_count = mesh.vertices_count;
    this->triangle_count = mesh.triangle_count;

    this->mesh_mode = mesh.mesh_mode;

    mesh.vbo = 0;
    mesh.instance_array_vbo = 0;
    mesh.ebo = 0;
    mesh.vao = 0;

    mesh.vertices_count = 0;
    mesh.triangle_count = 0;

    return *this;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &instance_array_vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}