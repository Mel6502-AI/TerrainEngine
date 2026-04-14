#pragma once
#include <string>
#include <vector>

#include "gl3w.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Shader.h"

class SkyBox {
public:
    explicit SkyBox(const std::string& skyboxDir);
    ~SkyBox();

    void render(const Camera& camera, const Shader& shader) const;
    void cleanup();

private:
    unsigned int loadCubemap(const std::string& dir) const;
    unsigned int VAO_ = 0;
    unsigned int textureID_ = 0;
};
