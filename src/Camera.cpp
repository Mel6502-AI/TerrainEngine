#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position) : position_(position) {}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::processInput(GLFWwindow* window) {
    float delta = 1.0f / 60.0f;  // Approximate — hook to delta time in main loop

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position_ += speed_ * delta * front_;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position_ -= speed_ * delta * front_;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position_ -= glm::normalize(glm::cross(front_, up_)) * speed_ * delta;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position_ += glm::normalize(glm::cross(front_, up_)) * speed_ * delta;

    // Mouse look via scroll (yaw/pitch)
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    static double prevX = mx, prevY = my;

    float dx = static_cast<float>(mx - prevX);
    float dy = static_cast<float>(my - prevY);
    prevX = mx;
    prevY = my;

    yaw_   += dx * 0.05f;
    pitch_ -= dy * 0.05f;
    pitch_  = glm::clamp(pitch_, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_));
    direction.y = glm::sin(glm::radians(pitch_));
    direction.z = glm::sin(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_));
    front_ = glm::normalize(direction);
}
