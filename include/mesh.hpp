#pragma once

#include <vector>

struct VertexData {
    int32_t packed_vertex_data;
};

enum class MeshMode {
    NON_INDEXED,
    INDEXED
};

struct Mesh {
    Mesh(const std::vector<VertexData>& vertices);
    Mesh(const std::vector<VertexData>& vertices, const std::vector<unsigned int>& indices);
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    // Won't be using copy constructor or copy assignment
    Mesh(const Mesh& mesh) = delete;
    Mesh& operator=(const Mesh& mesh) = delete;

    Mesh(Mesh&& mesh) noexcept;
    Mesh& operator=(Mesh&& mesh) noexcept;

    ~Mesh();

    unsigned int vbo = 0;
    unsigned int instance_array_vbo = 0;
    unsigned int ebo = 0;
    unsigned int vao = 0;

    unsigned int vertices_count = 0;
    unsigned int triangle_count = 0;

    MeshMode mesh_mode;
};