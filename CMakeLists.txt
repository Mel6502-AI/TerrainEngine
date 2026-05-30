cmake_minimum_required(VERSION 3.5.0)
project(terrainEngine)

set(CMAKE_CXX_STANDARD 14)

# --- Vendored dependencies (in-repo, see external/) ---
# external/glad/include also bundles GLM and stb_image.h
set(VENDOR_INCLUDE ${PROJECT_SOURCE_DIR}/external/glad/include)
set(GLFW_DIR       ${PROJECT_SOURCE_DIR}/external/glfw-3.4.bin.MACOS)
set(GLAD_SRC       ${PROJECT_SOURCE_DIR}/external/glad/src/glad.c)

include_directories(${VENDOR_INCLUDE} ${GLFW_DIR}/include ${PROJECT_SOURCE_DIR}/src)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(terrainEngine src/main.cpp ${GLAD_SRC})

# Resolve assets/shaders relative to the source tree, so the binary runs from any CWD.
target_compile_definitions(terrainEngine PRIVATE PROJECT_ROOT="${PROJECT_SOURCE_DIR}")

if(APPLE)
    # GLFW is shipped as a static archive (lib-universal/libglfw3.a); static linking on
    # macOS requires the Cocoa/IOKit/CoreVideo frameworks GLFW depends on.
    target_link_libraries(terrainEngine
        ${GLFW_DIR}/lib-universal/libglfw3.a
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo")
endif()
