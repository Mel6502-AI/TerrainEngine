# TerrainEngine

A small real-time 3D terrain scene rendered with modern OpenGL (3.3 core) on macOS.
You spawn above an island and can fly around it: heightmap-based terrain with detail
multi-texturing, a manually-mapped skybox, and an animated, reflective water plane.

See [`doc/TerrainDoc.md`](doc/TerrainDoc.md) for the original design notes.

## Features

- **Terrain** — mesh generated from `data/heightmap.bmp`, colored by `terrain-texture3.bmp`
  and overlaid with a tiled `detail.bmp` for close-up detail.
- **Skybox** — a high-resolution `GL_TEXTURE_CUBE_MAP` (sliced in-code from a single
  horizontal-cross PNG), sampled by direction so reflections mirror for free.
- **Water** — a blended plane that animates its texture coordinates each frame and shows an
  inverted (mirrored) reflection of the skybox and terrain.

## Requirements

- macOS (Apple Silicon or Intel)
- A C++14 compiler (Xcode command-line tools)
- [CMake](https://cmake.org/) ≥ 3.5 — `brew install cmake`

GLAD, GLFW (static), GLM and the stb_image **header** are vendored under `external/`, so only
CMake needs to be installed.

## Build

```sh
cmake -B build -S .
cmake --build build
```

The executable is written to the repository root as `./terrainEngine`.

## Run

```sh
./terrainEngine
```

Asset and shader paths are resolved from a compile-time `PROJECT_ROOT` define (set by CMake to
the source directory), so the binary can be launched from any working directory.

### Controls

| Input            | Action            |
|------------------|-------------------|
| `W` / `S`        | Move forward / back |
| `A` / `D`        | Strafe left / right |
| `Space`          | Move up           |
| `Left Shift`     | Move down         |
| Mouse            | Look around       |
| Scroll wheel     | Zoom (FOV)        |
| `Esc`            | Quit              |

## Project layout

```
TerrainEngine/
├── CMakeLists.txt
├── src/            main.cpp, camera.h, shader.h
├── shaders/        main and terrain vertex/fragment shaders (GLSL 330 core)
├── data/           heightmap, terrain/detail textures, SkyBox/
├── external/       vendored deps: glad/ (loader + GLM/stb headers), GLFW binary
└── doc/            TerrainDoc.md — original design notes
```

## Notes

- GLFW is linked as a static archive (`external/.../lib-universal/libglfw3.a`) together with the
  Cocoa / IOKit / CoreVideo / OpenGL frameworks (see `CMakeLists.txt`).
- The skybox is a 512×512-per-face cubemap loaded from `data/SkyBox/cubemap.png` (a 4×3
  horizontal cross). Swap that one PNG to change the sky.
- Skybox art: "Cloudy Skyboxes" by Screaming Brain Studios (CC0 / public domain).
