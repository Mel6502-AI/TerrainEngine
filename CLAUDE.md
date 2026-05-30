# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & run

```sh
cmake -B build -S .          # configure
cmake --build build          # builds ./terrainEngine into the repo root
./terrainEngine              # runs from any CWD (see PROJECT_ROOT below)
```

There are no tests, linter, or CI. The only target is `terrainEngine`.

The only external tool that must be installed is `cmake` (`brew install cmake`). Everything else
— GLAD, GLFW (static `.a`), GLM, and the stb_image header — is vendored under `external/`.

## Architecture

Single-file application (`src/main.cpp`, ~680 lines) plus two header-only helpers:
`shader.h` (compile/link a vertex+fragment pair from file paths) and `camera.h` (fly-camera with
Euler angles). It is a flat OpenGL 3.3 core program — no engine abstractions, no scene graph.

### Asset path convention (important)
All runtime files are loaded via `const std::string ROOT = PROJECT_ROOT;` where `PROJECT_ROOT` is
a compile-time string injected by CMake (`target_compile_definitions`) pointing at the source
tree. Every asset path is built as `ROOT + "/shaders/..."` or `ROOT + "/data/..."`. This is why
the binary runs from any working directory. Do **not** reintroduce relative or absolute paths —
add new assets under `data/` or `shaders/` and reference them through `ROOT`.

### The render loop is the architecture
The per-frame draw order in `main()` is deliberate and the visual result depends on it. Passes,
in order:
1. **Reflection skybox** (Y-flipped cube) — depth-tested, depth writes off.
2. **Normal skybox** — the above-water world.
3. **Second reflection skybox** below the water line — depth test off.
4. **Terrain** (`terrainShader`) — drawn twice: a mirrored copy (clip plane keeps `y <= water`)
   for the reflection, then the real terrain (clip plane keeps `y >= water`). Mirroring flips
   winding, so the code toggles `glFrontFace(GL_CW)` around the reflected draw. Clipping uses
   `gl_ClipDistance` driven by the `uClipPlane` uniform — `glEnable(GL_CLIP_DISTANCE0)` must wrap
   those draws.
5. **Water** quad (`shader`, faceType 5) — alpha-blended over the reflection; texture coords are
   scrolled each frame via the `uWaveShift` uniform for the wave animation.

Two shader programs are used: `shader` (main.vert/frag — skybox + water) and `terrainShader`
(terrain.vert/frag). Uniforms are looked up by name each frame (`glGetUniformLocation`); the C++
uniform names must match the GLSL exactly.

### Tunables
Scene scale and placement are compile-time `#define`s at the top of `main.cpp` (`SCALE`,
`SKYBOX_*`, `WATER_SPEED_*`, `XZ_SCALE`, `Y_SCALE`, `Y_OFFSET`, `DETAIL_TILING`,
`TERRAIN_Y_OFFSET`, fade distances). The terrain mesh is regenerated from the heightmap at
startup; changing the heightmap image changes the world.

## Gotchas

- `external/glad/include` bundles unused header sets (assimp, irrKlang, learnopengl) alongside the
  ones actually used. They are harmless but not dependencies.
- GLFW ships only as static archives here; linking pulls in the Cocoa/IOKit/CoreVideo/OpenGL
  frameworks (configured in `CMakeLists.txt`). The `*.dylib`/`*.a` gitignore rules have an
  exception for the vendored `libglfw3.a`.
- The skybox faces are 256×256 BMPs stretched over a large cube, so the sky looks soft/low-res
  by nature; only higher-resolution `data/SkyBox/*.bmp` images will sharpen it.
