#include "SkyBox.h"

#include <iostream>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

// Cube faces in order: +X, -X, +Y, -Y, +Z, -Z
static const std::vector<std::string> FACE_NAMES = {
    "SkyBox0.bmp", "SkyBox1.bmp", "SkyBox2.bmp",
    "SkyBox3.bmp", "SkyBox4.bmp", "SkyBox5.bmp"
};

SkyBox::SkyBox(const std::string& skyboxDir) : textureID_(loadCubemap(skyboxDir)) {
    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
    };

    unsigned int VBO;
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

SkyBox::~SkyBox() { cleanup(); }

void SkyBox::render(const Camera& camera, const Shader& shader) const {
    if (VAO_ == 0 || textureID_ == 0) return;

    glDepthFunc(GL_LEQUAL);
    shader.use();

    // Remove translation from view matrix
    glm::mat4 view = glm::mat4(glm::mat3(camera.viewMatrix()));
    shader.setMat4("view", view);
    shader.setMat4("projection",
                   glm::perspective(glm::radians(60.0f), 1024.0f / 768.0f,
                                    0.1f, 1000.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID_);
    shader.setInt("skybox", 0);

    glBindVertexArray(VAO_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

void SkyBox::cleanup() {
    if (VAO_) glDeleteVertexArrays(1, &VAO_);
    if (textureID_) glDeleteTextures(1, &textureID_);
    VAO_ = textureID_ = 0;
}

unsigned int SkyBox::loadCubemap(const std::string& dir) const {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    for (size_t i = 0; i < FACE_NAMES.size(); ++i) {
        std::string path = dir + "/" + FACE_NAMES[i];
        int w, h, ch;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i),
                         0, GL_RGB, w, h, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "SkyBox: failed to load face: " << path << "\n";
            // Fill with sky blue as fallback
            unsigned char fallback[3] = {135, 206, 235};
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i),
                         0, GL_RGB, 1, 1, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, fallback);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}
