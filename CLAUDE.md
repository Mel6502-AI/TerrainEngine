# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Terrain OpenGL Project

### Stack
- C++17, OpenGL 3.3 Core Profile (via GL3W, not GLEW), GLFW 3.4, GLM, stb_image

### Build and run
```bash
# Configure CMake (already configured in build/ directory)
cd build && cmake ..

# Build
make

# Run
./TerrainEngine
```

### Camera controls
- **WASD** - Move camera forward/left/backward/right
- **Mouse** - Look around (the camera tracks cursor position)
- **ESC** - Exit

### Architecture overview

The engine follows a classic OpenGL render loop pattern with three main rendering subsystems:

**Render flow:**
1. Skybox renders first with `glDepthMask(GL_FALSE)` to always appear behind
2. Terrain renders with depth testing enabled
3. Each frame clears color+depth, then draws skybox, then terrain

**Coordinate system:**
- Terrain mesh uses X-Z plane, Y is up (height)
- Heightmap grayscale [0-255] maps to Y values [0-maxHeight_]
- Camera starts at (0, 30, 60) looking toward origin

**Key classes:**
- `main.cpp` - GLFW init, window creation, render loop, subsystem initialization
- `TerrainRenderer` - Loads heightmap, generates VAO/VBO/EBO mesh, renders with shader
- `Shader` - Compiles vertex/fragment shaders, uniform setters (bool, int, float, vec3, mat4)
- `SkyBox` - Loads 6 cubemap faces (SkyBox0-5.bmp), renders without translation
- `Camera` - WASD movement, mouse look, generates view matrix

### Data formats

**Heightmaps** (`data/heightmap.bmp`):
- Grayscale BMP, any resolution
- Pixel value 0-255 → height 0-maxHeight_ (default 40.0)
- Loaded via stb_image with `STBI_grey`

**Terrain texture** (`data/terrain-texture3.bmp`):
- RGBA BMP, wraps GL_REPEAT
- Uses GL_LINEAR_MIPMAP_LINEAR filtering

**Skybox cubemap** (`data/SkyBox/`):
- 6 faces: SkyBox0.bmp through SkyBox5.bmp
- Face order: +X, -X, +Y, -Y, +Z, -Z
- RGB format, GL_CLAMP_TO_EDGE

### Shader pipeline

**Vertex attribute layout (terrain.vert):**
- Location 0: `vec3 aPos` - Position (x, height, z)
- Location 1: `vec2 aTexCoord` - UV coordinates
- Location 2: `vec3 aNormal` - Normal vector (currently all [0,1,0] placeholder)

**Terrain lighting (terrain.frag):**
- Blinn-Phong with ambient (0.25), diffuse (max(dot)), specular (pow(halfway, 32.0))
- Light direction fixed in shader, not passed from C++

**Skybox trick:**
- Vertex shader sets `gl_Position = pos.xyww` to force far plane depth
- Renders with `GL_LEQUAL` depth function instead of `GL_LESS`

### Mesh generation details

Terrain mesh is a regular grid:
- Vertices: `width * height` points with position, texcoord, normal
- Indices: 6 indices per grid cell (2 triangles)
- Vertex stride: 8 floats (3 pos + 2 tex + 3 normal = 32 bytes)

**Note:** Normals are currently placeholder `[0,1,0]` (pointing straight up). To compute proper normals:
- For each vertex, average face normals of adjacent triangles
- Or use central differences: `normal = normalize(cross(dx, dz))` where dx, dz are neighbor height deltas

### Dependencies and paths

**GLFW is bundled locally:**
- Headers: `/Users/melvyn/Downloads/glfw-3.4.bin.MACOS/include`
- Library: `/Users/melvyn/Downloads/glfw-3.4.bin.MACOS/lib-universal/libglfw.3.dylib`
- RPATH: `@executable_path/../Frameworks`

**GLM via Homebrew:** `/opt/homebrew/include`

**stb_image** and **gl3w** are single-header libraries in `src/`

### Common modifications

**Change terrain size:** Edit `width`/`height` loaded from heightmap in `TerrainRenderer::generateMesh()`

**Adjust camera speed:** Modify `speed_ = 15.0f` in `Camera` constructor

**Change FOV:** The 60.0f FOV is hardcoded in three places (`main.cpp`, `TerrainRenderer::render()`, `SkyBox::render()`)

**Enable proper lighting:** Pass `lightDir` and `viewPos` uniforms to terrain fragment shader

**Fix camera timing:** Currently uses hardcoded `1.0f/60.0f` delta in `Camera::processInput()` - should use actual frame time
