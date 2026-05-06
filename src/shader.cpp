#include "shader.hpp"

Shader::Shader() {
    ID = 0;
}

Shader::Shader(const char* vertex_path, const char* fragment_path, const char* geometry_path) {

    bool using_geometry_shader = std::string(geometry_path) != "";

    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertex_code;
    std::string fragment_code;
    std::string geometry_code;

    std::ifstream v_shader_file;
    std::ifstream f_shader_file;
    std::ifstream g_shader_file;

    // ensure ifstream objects can throw exceptions:
    v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    g_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // open files
        v_shader_file.open(vertex_path);
        f_shader_file.open(fragment_path);

        std::stringstream v_shader_stream, f_shader_stream;
        // read file's buffer contents into streams
        v_shader_stream << v_shader_file.rdbuf();
        f_shader_stream << f_shader_file.rdbuf();
        // close file handlers
        v_shader_file.close();
        f_shader_file.close();
        // convert stream into string
        vertex_code   = v_shader_stream.str();
        fragment_code = f_shader_stream.str();

        if (using_geometry_shader) {
            g_shader_file.open(geometry_path);
            std::stringstream g_shader_stream;
            g_shader_stream << g_shader_file.rdbuf();
            g_shader_file.close();
            geometry_code = g_shader_stream.str();
        }
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* v_shader_code = vertex_code.c_str();
    const char* f_shader_code = fragment_code.c_str();
    const char* g_shader_code = geometry_code.c_str();

    // Compile and configure shader program
    // ----------------------------------------------------------------------------------
    // Now for OpenGL to use the shader, need to compile it dynamically
    unsigned int vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &v_shader_code, nullptr);
    glCompileShader(vertex_shader);

    // Can check if the shader compiled correctly
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compiling fragment shader is similar to the vertex shader
    unsigned int fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &f_shader_code, nullptr);
    glCompileShader(fragment_shader);
    // Check for errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int geometry_shader;

    if (using_geometry_shader) {
        // Compiling geometry shader
        geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry_shader, 1, &g_shader_code, nullptr);
        glCompileShader(geometry_shader);
        // Check for errors
        glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometry_shader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // Link the shaders
    // To use our compiled shaders, need to link them to a shader program
    // Shader program will be the one to be called when doing render calls
    ID = glCreateProgram();
    glAttachShader(ID, vertex_shader); // Attach shader to program
    glAttachShader(ID, fragment_shader);
    if (using_geometry_shader) {
        glAttachShader(ID, geometry_shader);
    }
    glLinkProgram(ID); // Link attached shaders

    // Can check if something went wrong
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::CREATION\n" << infoLog << std::endl;
    }
    glDeleteShader(vertex_shader); // Don't need these shaders, can delete them now
    glDeleteShader(fragment_shader);

    if (using_geometry_shader) {
        glDeleteShader(geometry_shader);
    }
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::setUniformBlockBinding(const std::string& name, unsigned int binding) {
    glUniformBlockBinding(ID, glGetUniformBlockIndex(ID, name.c_str()), binding);
}

void ShaderLibrary::create(const std::string& name, const char* vertex_path,
                           const char* fragment_path, const char* geometry_path) {
    if (exists(name)) {
        std::cout << "Shader with this name already exists" << std::endl;
        return;
    }
    shaders[name] = Shader(vertex_path, fragment_path, geometry_path);
}

Shader& ShaderLibrary::get(const std::string& name) {
    if (!exists(name)) {
        std::cout << "Shader not found" << std::endl;
    }
    return shaders[name];
}

bool ShaderLibrary::exists(const std::string& name) const {
    return shaders.find(name) != shaders.end();
}
