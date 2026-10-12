#pragma once

#include <glm/glm.hpp>

#include <string>

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