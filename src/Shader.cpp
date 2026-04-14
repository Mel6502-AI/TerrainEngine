#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
    std::string vertSrc = loadFile(vertPath);
    std::string fragSrc = loadFile(fragPath);

    unsigned int vertShader = compileShader(vertSrc, GL_VERTEX_SHADER);
    unsigned int fragShader = compileShader(fragSrc, GL_FRAGMENT_SHADER);

    program_ = glCreateProgram();
    glAttachShader(program_, vertShader);
    glAttachShader(program_, fragShader);
    glLinkProgram(program_);

    int success;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program_, 512, nullptr, infoLog);
        std::cerr << "Shader program link error:\n" << infoLog << "\n";
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

Shader::~Shader() { destroy(); }

void Shader::use() const { glUseProgram(program_); }

void Shader::destroy() {
    if (program_) glDeleteProgram(program_);
    program_ = 0;
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(program_, name.c_str()),
                static_cast<int>(value));
}
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(program_, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(program_, name.c_str()), value);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program_, name.c_str()), 1,
                 glm::value_ptr(value));
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, name.c_str()), 1,
                       GL_FALSE, glm::value_ptr(value));
}

std::string Shader::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Shader: could not open file: " << path << "\n";
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::compileShader(const std::string& src, GLenum type) {
    unsigned int shader = glCreateShader(type);
    const char* cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error (" << (type == GL_VERTEX_SHADER ? "vert" : "frag")
                  << "):\n" << infoLog << "\n";
    }
    return shader;
}
