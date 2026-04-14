#include "TerrainRenderer.h"

#include <fstream>
#include <iostream>
#include <vector>

TerrainRenderer::TerrainRenderer(const std::string& heightmapPath,
                                 const std::string& texturePath)
    : textureID_(loadTexture(texturePath)) {

    int width, height, channels;
    unsigned char* img = stbi_load(heightmapPath.c_str(), &width, &height,
                                   &channels, STBI_grey);
    if (!img) {
        std::cerr << "TerrainRenderer: failed to load heightmap: "
                  << heightmapPath << "\n";
        return;
    }

    // Convert to float height values [0..255] -> [0..maxHeight_]
    std::vector<float> heights(width * height);
    for (int i = 0; i < width * height; ++i) {
        heights[i] = (img[i] / 255.0f) * maxHeight_;
    }
    stbi_image_free(img);

    generateMesh(width, height, heights.data());

    std::cout << "TerrainRenderer: mesh " << width << "x" << height
              << " ready (" << indexCount_ << " indices)\n";
}

TerrainRenderer::~TerrainRenderer() { cleanup(); }

void TerrainRenderer::render(const Camera& camera, const Shader& shader) const {
    if (VAO_ == 0) return;

    shader.use();
    shader.setMat4("view", camera.viewMatrix());
    shader.setMat4("projection",
                   glm::perspective(glm::radians(60.0f), 1024.0f / 768.0f,
                                    0.1f, 1000.0f));

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID_);
    shader.setInt("terrainTexture", 0);

    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void TerrainRenderer::cleanup() {
    if (VAO_) glDeleteVertexArrays(1, &VAO_);
    if (VBO_) glDeleteBuffers(1, &VBO_);
    if (EBO_) glDeleteBuffers(1, &EBO_);
    if (textureID_) glDeleteTextures(1, &textureID_);
    VAO_ = VBO_ = EBO_ = textureID_ = 0;
}

void TerrainRenderer::generateMesh(int width, int height,
                                   const float* heightData) {
    std::vector<float> vertices;
    vertices.reserve(static_cast<size_t>(width) * height * 8);  // pos(3)+tex(2)+norm(3)

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float h = heightData[z * width + x];

            // Position
            vertices.push_back(static_cast<float>(x)); vertices.push_back(h);
            vertices.push_back(static_cast<float>(z));

            // Tex coord
            vertices.push_back(static_cast<float>(x) / width);
            vertices.push_back(static_cast<float>(z) / height);

            // Normal (placeholder — compute flat normals from neighbours below)
            vertices.push_back(0.0f); vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
    }

    std::vector<unsigned int> indices;
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            unsigned int topLeft     = z * width + x;
            unsigned int topRight    = topLeft + 1;
            unsigned int bottomLeft  = topLeft + width;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);     indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);  indices.push_back(bottomRight);
        }
    }

    indexCount_ = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // TexCoord (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normal (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          reinterpret_cast<void*>(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

unsigned int TerrainRenderer::loadTexture(const std::string& path) const {
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (!data) {
        std::cerr << "TerrainRenderer: failed to load texture: " << path << "\n";
        return 0;
    }

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}
