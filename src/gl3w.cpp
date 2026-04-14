/*
 * gl3w - OpenGL 3.3 Core Profile function loader for macOS
 * Simple wrapper around glfwGetProcAddress
 */
#include "gl3w.h"
#include <GLFW/glfw3.h>

int gl3wInit(void) {
    return 1;
}

void* gl3wGetProcAddress(const char* name) {
    return (void*)glfwGetProcAddress(name);
}
