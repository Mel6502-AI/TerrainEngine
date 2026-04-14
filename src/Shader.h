#pragma once

#include <string>

#include "gl3w.h"
#include <glm/glm.hpp>

class Shader {
public:
    Shader() = default;
    explicit Shader(const std::string& vertPath, const std::string& fragPath);
    ~Shader();

    void use() const;
    void destroy();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    static std::string loadFile(const std::string& path);
    static unsigned int compileShader(const std::string& src, GLenum type);

    unsigned int program_ = 0;
};
