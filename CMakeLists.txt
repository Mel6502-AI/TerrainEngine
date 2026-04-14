cmake_minimum_required(VERSION 3.16)
project(TerrainEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Local GLFW (pre-built)
set(GLFW_INCLUDE_DIR "/Users/melvyn/Downloads/glfw-3.4.bin.MACOS/include")
set(GLFW_LIBRARY "/Users/melvyn/Downloads/glfw-3.4.bin.MACOS/lib-universal/libglfw.3.dylib")
set(GLFW_RPATH "@executable_path/../Frameworks")

# System GLM
set(GLM_INCLUDE_DIR "/opt/homebrew/include")

include_directories(
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${GLFW_LIBRARY}
    INSTALL_RPATH ${GLFW_RPATH}
)

add_executable(TerrainEngine
    src/main.cpp
    src/TerrainRenderer.cpp
    src/SkyBox.cpp
    src/Camera.cpp
    src/Shader.cpp
    src/gl3w.cpp
    src/stb_image_impl.cpp
)

target_link_libraries(TerrainEngine
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
)
