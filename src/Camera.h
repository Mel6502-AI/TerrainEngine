#pragma once

#include "gl3w.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:
    explicit Camera(glm::vec3 position);

    glm::mat4 viewMatrix() const;

    void processInput(GLFWwindow* window);

private:
    glm::vec3 position_;
    glm::vec3 front_    = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up_       = glm::vec3(0.0f, 1.0f, 0.0f);
    float yaw_   = -90.0f;
    float pitch_ = -20.0f;
    float speed_ = 15.0f;
};
