#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "mesh.hpp"

class ResourceManager {

public:
    void createMesh(const std::string& name, const std::vector<VertexData>& vertices);
    void createMesh(const std::string& name, const std::vector<VertexData>& vertices, 
                    const std::vector<unsigned int>& indices);
    void createMesh(const std::string& name, const std::vector<float>& vertices,
                    const std::vector<unsigned int>& indices);
    
    void createMeshFromObj(const std::string& name, const std::string& file_path);

    Mesh* tryGetMesh(const std::string& name);

    bool doesMeshExist(const std::string& name) const;

private:
	std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
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