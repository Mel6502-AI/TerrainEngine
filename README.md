# TerrainEngine

A small real-time 3D terrain scene rendered with modern OpenGL (3.3 core) on macOS.
You spawn above an island and can fly around it: heightmap-based terrain with detail
multi-texturing, a manually-mapped skybox, and an animated, reflective water plane, with
a live camera-position HUD.

See [`doc/TerrainDoc.md`](doc/TerrainDoc.md) for the original design notes.

## Features

- **Terrain** — mesh generated from `data/heightmap.bmp`, colored by `terrain-texture3.bmp`
  and overlaid with a tiled `detail.bmp` for close-up detail.
- **Skybox** — six BMP faces mapped onto the inside of a cube (no GL cubemap, so faces of
  different resolutions can be mixed).
- **Water** — a blended plane that animates its texture coordinates each frame and shows an
  inverted (mirrored) reflection of the skybox and terrain.
- **HUD** — FreeType-rendered text showing the live camera X/Y/Z position.

## Requirements

- macOS (Apple Silicon or Intel)
- A C++14 compiler (Xcode command-line tools)
- [CMake](https://cmake.org/) ≥ 3.5 — `brew install cmake`
- [FreeType](https://freetype.org/) — `brew install freetype`

GLAD, GLFW (static), GLM and the FreeType/stb_image **headers** are vendored under
`external/`, so only CMake and the FreeType library need to be installed.

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
├── shaders/        main, terrain, and text vertex/fragment shaders (GLSL 330 core)
├── data/           heightmap, terrain/detail textures, SkyBox/, fonts/Monaco.ttf
├── external/       vendored deps: glad/ (loader + GLM/FreeType/stb headers), GLFW binary
└── doc/            TerrainDoc.md — original design notes
```

## Notes

- `data/fonts/Monaco.ttf` is vendored so the HUD works out of the box. To swap fonts, drop a TTF
  in `data/fonts/` and update the `FT_New_Face` path in `src/main.cpp`.
- GLFW is linked as a static archive (`external/.../lib-universal/libglfw3.a`) together with the
  Cocoa / IOKit / CoreVideo / OpenGL frameworks (see `CMakeLists.txt`).
- On launch macOS may log `UNSUPPORTED ... using zero texture` once — a benign legacy-GL
  warning that does not affect rendering.
