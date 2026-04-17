cmake_minimum_required(VERSION 3.5.0)
project(Wall)

set(CMAKE_CXX_STANDARD 14)
set(GLAD_H /Users/melvyn/computer_graphics/glad/include)
set(GLFW_H /Users/melvyn/computer_graphics/glfw-3.4.bin.MACOS/include)
set(GLM_H /opt/homebrew/include/glm)
set(STBI_H /Users/melvyn/computer_graphics)
include_directories(${GLAD_H} ${GLFW_H} ${GLM_H} ${STBI_H})

set(GLFW_LINK /Users/melvyn/computer_graphics/glfw-3.4.bin.MACOS/lib-universal/libglfw.3.dylib)
link_libraries(${GLFW_LINK})

set(SOURCE_MAIN main.cpp)
set(SOURCE_GLAD /Users/melvyn/computer_graphics/glad/src/glad.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(wall ${SOURCE_MAIN} ${SOURCE_GLAD})

if (APPLE)
    target_link_libraries(wall "-framework OpenGL")
endif()
