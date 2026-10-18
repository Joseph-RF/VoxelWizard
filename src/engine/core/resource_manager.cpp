#include "resource_manager.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

ResourceManager::ResourceManager() {
    for(int i = 0; i < MAX_MESHES; ++i) {
        available_mesh_ids.push(i);
    }
    for(int i = 0; i < MAX_SHADERS; ++i) {
        available_shader_ids.push(i);
    }
}

void ResourceManager::createMesh(const std::string &name, const std::vector<VertexData> &vertices) {
    if(available_mesh_ids.empty()) {
        std::cout << "At max capacity for meshes" << std::endl;
        return;
    }
    if (doesMeshExist(name)) {
        std::cout << "Mesh with this name already exists" << std::endl;
        return;
    }
    meshes.insert({ name, std::make_unique<Mesh>(vertices) });
    mesh_name_to_id.insert({name, available_mesh_ids.front()});
    mesh_id_to_name.insert({available_mesh_ids.front(), name});
    available_mesh_ids.pop();
}

void ResourceManager::createMesh(const std::string& name, const std::vector<VertexData>& vertices,
							     const std::vector<unsigned int>& indices) {
    if(available_mesh_ids.empty()) {
        std::cout << "At max capacity for meshes" << std::endl;
        return;
    }
    if (doesMeshExist(name)) {
        std::cout << "Mesh with this name already exists" << std::endl;
        return;
    }
    meshes.insert({name, std::make_unique<Mesh>(vertices, indices)});
    mesh_name_to_id.insert({name, available_mesh_ids.front()});
    mesh_id_to_name.insert({available_mesh_ids.front(), name});
    available_mesh_ids.pop();
}

void ResourceManager::createMeshFromObj(const std::string& name, const std::string& file_path) {
    if(available_mesh_ids.empty()) {
        std::cout << "At max capacity for meshes" << std::endl;
        return;
    }
    if (doesMeshExist(name)) {
        std::cout << "Mesh with this name already exists. Could not load mesh from obj file." << std::endl;
        return;
    }

    std::vector<float> vertex_positions;
    std::vector<float> vertex_normals;
    std::vector<float> vertex_colours;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::unordered_map<std::pair<unsigned int, unsigned int>, unsigned int, HashUIntPair> used_index_combinations;

    std::ifstream my_file(file_path);

    if (my_file.is_open()) {
        std::string line;
        std::string word;
        int index_counter = 0;

        while (std::getline(my_file, line)) {
            std::stringstream ss(line);
            ss >> word;

            if (word == "v") {

                // Expecting vertex position and colour to be on the same line

                ss >> word;
                vertex_positions.push_back(std::stof(word));
                ss >> word;
                vertex_positions.push_back(std::stof(word));
                ss >> word;
                vertex_positions.push_back(std::stof(word));
                ss >> word;
                vertex_colours.push_back(std::stof(word));
                ss >> word;
                vertex_colours.push_back(std::stof(word));
                ss >> word;
                vertex_colours.push_back(std::stof(word));
            }
            else if (word == "vn") {
                ss >> word;
                vertex_normals.push_back(std::stof(word));
                ss >> word;
                vertex_normals.push_back(std::stof(word));
                ss >> word;
                vertex_normals.push_back(std::stof(word));
            }
            else if (word == "f") {
                for (int i = 0; i < 3; ++i) {
                    ss >> word;
                    unsigned int position_colour_index;
                    unsigned int normal_index;

                    // Vertex positions and colour
                    size_t word_index = word.find("/");
                    if (word_index == std::string::npos) {
                        std::cout << "Model loading line for face elements encountered an error" << std::endl;
                        break;
                    }

                    position_colour_index = std::stoi(word.substr(0, word_index));
                    word.erase(0, word_index + 1);

                    // Discarding the texture index
                    word_index = word.find("/");
                    if (word_index == std::string::npos) {
                        std::cout << "Model loading line for face elements encountered an error" << std::endl;
                        break;
                    }
                    word.erase(0, word_index + 1);

                    // Vertex normal
                    normal_index = std::stoi(word);

                    // Now check if the position-colour and normal index combination is unique
                    std::pair<unsigned int, unsigned int> index_combination(position_colour_index, normal_index);
                    if (used_index_combinations.count(index_combination) == 0) {
                        used_index_combinations.emplace(index_combination, index_counter);

                        vertices.push_back(vertex_positions[3 * (position_colour_index - 1)]);     // x
                        vertices.push_back(vertex_positions[3 * (position_colour_index - 1) + 1]); // y
                        vertices.push_back(vertex_positions[3 * (position_colour_index - 1) + 2]); // z

                        vertices.push_back(vertex_colours[3 * (position_colour_index - 1)]);     // x
                        vertices.push_back(vertex_colours[3 * (position_colour_index - 1) + 1]); // y
                        vertices.push_back(vertex_colours[3 * (position_colour_index - 1) + 2]); // z

                        vertices.push_back(vertex_normals[3 * (normal_index - 1)]);     // x
                        vertices.push_back(vertex_normals[3 * (normal_index - 1) + 1]); // y
                        vertices.push_back(vertex_normals[3 * (normal_index - 1) + 2]); // z

                        indices.push_back(index_counter);

                        index_counter++;
                    }
                    else {
                        indices.push_back(used_index_combinations.at(index_combination));
                    }
                }
            }
        }
        my_file.close();
    }
    else {
        std::cout << "Unable to open file" << std::endl;
        return;
    }

    meshes.insert({ name, std::make_unique<Mesh>(vertices, indices) });
    mesh_name_to_id.insert({name, available_mesh_ids.front()});
    mesh_id_to_name.insert({available_mesh_ids.front(), name});
    available_mesh_ids.pop();
}

Mesh* ResourceManager::tryGetMesh(const std::string& name) {

    // Ensure this pointer is never saved by ANYTHING
    // Usage should be: Use and discard ALWAYS

    if (!doesMeshExist(name)) {
        std::cout << "Mesh not found (with name)" << std::endl;
        return nullptr;
    }
    return meshes.at(name).get();
}

Mesh *ResourceManager::tryGetMesh(uint16_t id) {

    // Ensure this pointer is never saved by ANYTHING
    // Usage should be: Use and discard ALWAYS

    if(!doesMeshExist(id)) {
        std::cout << "Mesh not found (with id)" << std::endl;
        return nullptr;
    }
    return meshes.at(mesh_id_to_name.at(id)).get();
}

void ResourceManager::createShader(const std::string &name, const char *vertex_path, 
                                   const char *fragment_path, const char *geometry_path) {
    if(available_shader_ids.empty()) {
        std::cout << "At max capacity for shaders" << std::endl;
        return;
    }
    if (doesShaderExist(name)) {
        std::cout << "Shader with this name already exists" << std::endl;
        return;
    }
    shaders.insert({name, Shader(vertex_path, fragment_path, geometry_path) });
    shader_name_to_id.insert({name, available_shader_ids.front()});
    shader_id_to_name.insert({available_shader_ids.front(), name});
    available_shader_ids.pop();
}

Shader &ResourceManager::getShader(const std::string &name) {
    if (!doesShaderExist(name)) {
        std::cout << "Shader not found (with name)" << std::endl;
    }
    return shaders.at(name);
}

Shader &ResourceManager::getShader(uint16_t id) {
    if (!doesShaderExist(id)) {
        std::cout << "Shader not found (with id)" << std::endl;
    }
    return shaders.at(shader_id_to_name.at(id));
}

std::string ResourceManager::getMeshName(uint16_t id) {
    if (!doesMeshExist(id)) {
        std::cout << "Mesh not found (with id)" << std::endl;
        return std::string();
    }
    return mesh_id_to_name[id];
}

uint16_t ResourceManager::getMeshID(const std::string &name) {
    if (!doesMeshExist(name)) {
        std::cout << "Mesh not found (with name)" << std::endl;
        return 0;
    }
    return mesh_name_to_id[name];
}

std::string ResourceManager::getShaderName(uint16_t id) {
    if (!doesShaderExist(id)) {
        std::cout << "Shader not found (with id)" << std::endl;
        return std::string();
    }
    return shader_id_to_name[id];
}

uint16_t ResourceManager::getShaderID(const std::string &name) {
    if (!doesShaderExist(name)) {
        std::cout << "Shader not found (with name)" << std::endl;
        return 0;
    }
    return shader_name_to_id[name];
}

bool ResourceManager::doesMeshExist(const std::string& name) const {
    return meshes.find(name) != meshes.end();
}

bool ResourceManager::doesMeshExist(const uint16_t id) const {
    return mesh_id_to_name.find(id) != mesh_id_to_name.end();
}

bool ResourceManager::doesShaderExist(const std::string &name) const {
    return shaders.find(name) != shaders.end();
}

bool ResourceManager::doesShaderExist(const uint16_t id) const {
    return shader_id_to_name.find(id) != shader_id_to_name.end();
}
