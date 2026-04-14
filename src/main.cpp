#include "gl3w.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Camera.h"
#include "Shader.h"
#include "SkyBox.h"
#include "TerrainRenderer.h"

static constexpr int WINDOW_WIDTH  = 1024;
static constexpr int WINDOW_HEIGHT = 768;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    // Configure OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "TerrainEngine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL function pointers via GLAD
    if (!gl3wInit()) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version:  " << glGetString(GL_VERSION)   << "\n";

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Build shader programs
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

    // Initialize subsystems
    Camera camera(glm::vec3(0.0f, 30.0f, 60.0f));
    TerrainRenderer terrainRenderer("data/heightmap.bmp", "data/terrain-texture3.bmp");
    SkyBox skybox("data/SkyBox/");

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Handle camera input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        camera.processInput(window);

        // Clear buffers
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw skybox first
        glDepthMask(GL_FALSE);
        skybox.render(camera, skyboxShader);
        glDepthMask(GL_TRUE);

        // Draw terrain
        terrainRenderer.render(camera, terrainShader);

        glfwSwapBuffers(window);
    }

    // Cleanup
    terrainRenderer.cleanup();
    skybox.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
