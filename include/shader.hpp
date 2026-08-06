#pragma once


#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

class Shader {
public:
    // the program ID
    unsigned int ID;

    Shader();
    // constructor reads and builds the shader
    Shader(const char* vertex_path, const char* fragment_path, const char* geometry_path);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setUniformBlockBinding(const std::string& name, unsigned int binding);
};

class ShaderLibrary {
    // Thanks for the idea TheCherno
public:
    void create(const std::string& name, const char* vertex_path, const char* fragment_path,
                const char* geometry_path);

    Shader& get(const std::string& name);

private:
    bool exists(const std::string& name) const;

    std::unordered_map<std::string, Shader> shaders;
};