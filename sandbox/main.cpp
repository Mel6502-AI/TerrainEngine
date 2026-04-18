#define XZ_SCALE 0.25f
#define Y_SCALE  20.0f
#define DETAIL_TILING 32.0f

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "shader.h"
#include "camera.h"

GLuint WIDTH = 800, HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 15.0f, 40.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void Do_Movement();

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Terrain Sandbox", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);

    // Load heightmap
    const char* heightmapPath = "data/heightmap.bmp";
    std::cout << "Loading heightmap: " << heightmapPath << std::endl;

    int w, h, channels;
    unsigned char* image = stbi_load(heightmapPath, &w, &h, &channels, 1);
    if (!image) {
        std::cout << "Failed to load heightmap: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    std::cout << "Heightmap loaded: " << w << " x " << h << std::endl;

    // Build terrain mesh
    std::vector<float> vertices;
    std::vector<GLuint> indices;

    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            float x = (i - w / 2.0f) * XZ_SCALE;
            float y = (image[j * w + i] / 255.0f) * Y_SCALE;
            float z = (j - h / 2.0f) * XZ_SCALE;
            float u = i / (w - 1.0f);
            float v = j / (h - 1.0f);
            vertices.insert(vertices.end(), {x, y, z, u, v});
        }
    }

    for (int j = 0; j < h - 1; ++j) {
        for (int i = 0; i < w - 1; ++i) {
            GLuint i00 = j * w + i;
            GLuint i10 = j * w + (i + 1);
            GLuint i01 = (j + 1) * w + i;
            GLuint i11 = (j + 1) * w + (i + 1);
            indices.insert(indices.end(), {
                i00, i10, i11,
                i00, i11, i01
            });
        }
    }

    stbi_image_free(image);

    // Load color texture
    GLuint colorTexID;
    int texW, texH, texChannels;
    const char* colorTexPath = "data/terrain-texture3.bmp";
    std::cout << "Loading color texture: " << colorTexPath << std::endl;
    unsigned char* colorImage = stbi_load(colorTexPath, &texW, &texH, &texChannels, 3);
    if (!colorImage) {
        std::cout << "Failed to load color texture: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    std::cout << "Color texture loaded: " << texW << " x " << texH << std::endl;

    glGenTextures(1, &colorTexID);
    glBindTexture(GL_TEXTURE_2D, colorTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, colorImage);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(colorImage);

    // Load detail texture
    GLuint detailTexID;
    int detW, detH, detChannels;
    const char* detailTexPath = "data/detail.bmp";
    std::cout << "Loading detail texture: " << detailTexPath << std::endl;
    unsigned char* detailImage = stbi_load(detailTexPath, &detW, &detH, &detChannels, 3);
    if (!detailImage) {
        std::cout << "Failed to load detail texture: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    std::cout << "Detail texture loaded: " << detW << " x " << detH << std::endl;

    glGenTextures(1, &detailTexID);
    glBindTexture(GL_TEXTURE_2D, detailTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, detW, detH, 0, GL_RGB, GL_UNSIGNED_BYTE, detailImage);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(detailImage);

    GLuint vertCount = vertices.size() / 5;
    GLuint triCount = indices.size() / 3;
    std::cout << "Vertex count: " << vertCount << std::endl;
    std::cout << "Triangle count: " << triCount << std::endl;

    // Create VAO/VBO/EBO
    GLuint terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    Shader terrainShader("sandbox/terrain.vert.glsl", "sandbox/terrain.frag.glsl");

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        Do_Movement();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrainShader.Use();

        glm::mat4 model(1);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f);

        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(terrainShader.Program, "uMinHeight"), 0.0f);
        glUniform1f(glGetUniformLocation(terrainShader.Program, "uMaxHeight"), Y_SCALE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTexID);
        glUniform1i(glGetUniformLocation(terrainShader.Program, "uColorTex"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, detailTexID);
        glUniform1i(glGetUniformLocation(terrainShader.Program, "uDetailTex"), 1);
        glUniform1f(glGetUniformLocation(terrainShader.Program, "uDetailTiling"), DETAIL_TILING);

        glBindVertexArray(terrainVAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void Do_Movement() {
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_Q]) camera.ProcessKeyboard(DOWN, deltaTime);
    if (keys[GLFW_KEY_E]) camera.ProcessKeyboard(UP, deltaTime);
    if (keys[GLFW_KEY_SPACE]) camera.ProcessKeyboard(UP, deltaTime);
    if (keys[GLFW_KEY_LEFT_SHIFT]) camera.ProcessKeyboard(DOWN, deltaTime);
    if (keys[GLFW_KEY_RIGHT_SHIFT]) camera.ProcessKeyboard(DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
    if (action == GLFW_PRESS) keys[key] = true;
    if (action == GLFW_RELEASE) keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}