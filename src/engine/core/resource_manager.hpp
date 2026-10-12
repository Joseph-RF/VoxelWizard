#pragma once

#include "engine/graphics/mesh.hpp"
#include "engine/graphics/shader.hpp"

#include <unordered_map>
#include <string>
#include <memory>
#include <queue>

class ResourceManager {

public:
    ResourceManager();

    // Meshes
    // ------------------------------------------
    void createMesh(const std::string& name, const std::vector<VertexData>& vertices);
    void createMesh(const std::string& name, const std::vector<VertexData>& vertices, 
                    const std::vector<unsigned int>& indices);
    void createMesh(const std::string& name, const std::vector<float>& vertices,
                    const std::vector<unsigned int>& indices);
    
    void createMeshFromObj(const std::string& name, const std::string& file_path);

    Mesh* tryGetMesh(const std::string& name);
    Mesh* tryGetMesh(uint32_t id);

    // Shaders
    // ------------------------------------------
    void createShader(const std::string& name, const char* vertex_path, const char* fragment_path,
                      const char* geometry_path);

    Shader& getShader(const std::string& name);
    Shader& getShader(uint32_t id);

    // Utility
    // ------------------------------------------
    std::string getMeshName(uint32_t id);
    uint32_t getMeshID(const std::string& name);

    std::string getShaderName(uint32_t id);
    uint32_t getShaderID(const std::string& name);

private:

    // Meshes
    // ------------------------------------------
	std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::unordered_map<std::string, uint32_t> mesh_name_to_id;
    std::unordered_map<uint32_t, std::string> mesh_id_to_name;

    static const uint32_t MAX_MESHES = 1000;
    std::queue<uint32_t> available_mesh_ids;

    bool doesMeshExist(const std::string& name) const;
    bool doesMeshExist(const uint32_t id) const;

    // Shaders
    // ------------------------------------------
    std::unordered_map<std::string, Shader> shaders;
    std::unordered_map<std::string, uint32_t> shader_name_to_id;
    std::unordered_map<uint32_t, std::string> shader_id_to_name;

    static const uint32_t MAX_SHADERS = 100;
    std::queue<uint32_t> available_shader_ids;

    bool doesShaderExist(const std::string& name) const;
    bool doesShaderExist(const uint32_t id) const;
};

struct HashUIntPair {
public:
    std::size_t operator()(const std::pair<unsigned int, unsigned int>& p) const noexcept {
        std::size_t h1 = std::hash<int>{}(p.first);
        std::size_t h2 = std::hash<int>{}(p.second);
        // boost-style combine
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};