cmake_minimum_required(VERSION 3.5.0)
project(terrainEngine)

set(CMAKE_CXX_STANDARD 14)

# --- Vendored dependencies (in-repo, see external/) ---
# external/glad/include also bundles GLM, FreeType headers, and stb_image.h
set(VENDOR_INCLUDE ${PROJECT_SOURCE_DIR}/external/glad/include)
set(GLFW_DIR       ${PROJECT_SOURCE_DIR}/external/glfw-3.4.bin.MACOS)
set(GLAD_SRC       ${PROJECT_SOURCE_DIR}/external/glad/src/glad.c)

include_directories(${VENDOR_INCLUDE} ${GLFW_DIR}/include ${PROJECT_SOURCE_DIR}/src)

# --- FreeType (Homebrew on macOS; headers are vendored but the library is not) ---
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

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(terrainEngine src/main.cpp ${GLAD_SRC})

# Resolve assets/shaders relative to the source tree, so the binary runs from any CWD.
target_compile_definitions(terrainEngine PRIVATE PROJECT_ROOT="${PROJECT_SOURCE_DIR}")

if(APPLE)
    # GLFW is shipped as a static archive (lib-universal/libglfw3.a); static linking on
    # macOS requires the Cocoa/IOKit/CoreVideo frameworks GLFW depends on.
    target_link_libraries(terrainEngine
        ${GLFW_DIR}/lib-universal/libglfw3.a
        ${FREETYPE_LIBRARY}
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo")
endif()
