#pragma once
#include <string>

#include <stb_image.h>

#include "gl3w.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Shader.h"

class TerrainRenderer {
public:
    explicit TerrainRenderer(const std::string& heightmapPath,
                             const std::string& texturePath);
    ~TerrainRenderer();

    void render(const Camera& camera, const Shader& shader) const;
    void cleanup();

private:
    void generateMesh(int width, int height, const float* heightData);
    unsigned int loadTexture(const std::string& path) const;

    unsigned int VAO_ = 0;
    unsigned int VBO_ = 0;
    unsigned int EBO_ = 0;
    unsigned int textureID_ = 0;
    unsigned int indexCount_ = 0;
    float maxHeight_ = 40.0f;
};
