cmake_minimum_required(VERSION 3.5.0)
project(terrainEngine)

set(CMAKE_CXX_STANDARD 14)
set(GLAD_H /Users/melvyn/computer_graphics/glad/include)
set(GLFW_H /Users/melvyn/computer_graphics/glfw-3.4.bin.MACOS/include)
set(GLM_H /opt/homebrew/include/glm)
set(STBI_H /Users/melvyn/computer_graphics)
include_directories(${GLAD_H} ${GLFW_H} ${GLM_H} ${STBI_H})

set(GLFW_LINK /Users/melvyn/computer_graphics/glfw-3.4.bin.MACOS/lib-universal/libglfw.3.dylib)
link_libraries(${GLFW_LINK})

# FreeType setup for macOS
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/freetype")
        set(FREETYPE_INCLUDE_DIR "/opt/homebrew/opt/freetype/include/freetype2")
        set(FREETYPE_LIBRARY "/opt/homebrew/opt/freetype/lib/libfreetype.dylib")
    elseif(EXISTS "/usr/local/opt/freetype")
        set(FREETYPE_INCLUDE_DIR "/usr/local/opt/freetype/include/freetype2")
        set(FREETYPE_LIBRARY "/usr/local/opt/freetype/lib/libfreetype.dylib")
    else()
        set(FREETYPE_INCLUDE_DIR "/opt/homebrew/include/freetype2")
        set(FREETYPE_LIBRARY "/usr/X11/lib/libfreetype.6.dylib")
    endif()
    include_directories(${FREETYPE_INCLUDE_DIR})
endif()

set(SOURCE_MAIN main.cpp)
set(SOURCE_GLAD /Users/melvyn/computer_graphics/glad/src/glad.c)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(terrainEngine ${SOURCE_MAIN} ${SOURCE_GLAD})


if(APPLE)
    target_link_libraries(terrainEngine "-framework OpenGL" ${FREETYPE_LIBRARY})
endif()

# Terrain Sandbox - standalone terrain prototype executable
set(SOURCE_SANDBOX sandbox/main.cpp)
add_executable(TerrainSandbox ${SOURCE_SANDBOX} ${SOURCE_GLAD})
target_include_directories(TerrainSandbox PRIVATE ${CMAKE_SOURCE_DIR} ${STBI_H})
if(APPLE)
    target_link_libraries(TerrainSandbox "-framework OpenGL")
endif()
