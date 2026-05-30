# Terrain Engine
*Minjing Yu*

---

## Contents

1. Required Tasks
2. Detail Description
   - 2.1 Skybox
     - 2.1.1 Images used for texturing
     - 2.1.2 Parameters of the box
     - 2.1.3 Erasing edges
   - 2.2 Water wave
   - 2.3 Terrain Model
     - 2.3.1 Loading height information
     - 2.3.2 Texture mapping
     - 2.3.3 Put it in the sea
   - 2.4 Interaction
3. Bonus

---

## 1 Required Tasks

The goal of this project is to construct a 3D scene and wander in it.

To finish this project, the following techniques are necessary:

- **Texture** (for constructing skybox, water wave and the terrain).
- **Model-View transformation** (for control movement in the scene).
- **Load terrain data** from an image and render it.

---

## 2 Detail Description

This part will talk about the implementation in detail.

### 2.1 Skybox

Let's build up the scene by starting with the skybox, which includes blue sky with clouds and the sun. Besides, we will also create the flowing sea and the inverted reflection of the sky.

#### 2.1.1 Images used for texturing

Generally, to build up the sky, we basically just use images taken from the same position to different directions (front, back, left, right, top and bottom) and map them to the inside of a box, which we denote as *skybox*. And if our eyes are in the box, it will look like a panoramic view.

The Cubemap, as a texture generation method that OpenGL provides, makes building up a skybox easier. But in this project, it may be wiser to avoid using it for the following reasons:

1. 6 images are needed to build up a cubemap, and in our case we only have 5 because we'll leave the bottom one for the sea surface. The water wave texture has a different resolution from the others, so it cannot be applied directly as part of a cubemap.
2. The cubemap in OpenGL is mainly used to create a reflection mirrored effect on the surface of an object, which in our project is not the purpose.

So we can just manually use 5 2D textures and map them onto the inner-faces of a cube.

#### 2.1.2 Parameters of the box

As a skybox for a large scene, the skybox may be quite large, which you need to adjust yourself to get the most comfortable parameters. Another thing to notice is the ratio among the length, width and height of the skybox.

With a 1×1×1 textured skybox, the sun looks very "thin" because the original image looks just like that (probably because the camera taking the pictures has a large FOV). To make it better, we need to stretch the skybox a little. There is no standard for this — it's your call to adjust the ratio to make the scene look normal.

#### 2.1.3 Erasing edges

A problem that may occur is that the edges of the skybox are shown explicitly and reveal that the scene is actually just a box, which is obviously not what we want. This problem is related to the **wrapping mode of texture** you set.

```c
void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
```

With the texture target (e.g. `GL_TEXTURE_2D`) and the texture coordinate wrapping (e.g. `GL_TEXTURE_WRAP_S` and `GL_TEXTURE_WRAP_T`), we have four parameters:

| Parameter | Description |
|---|---|
| `GL_REPEAT` | Repeats the texture |
| `GL_CLAMP` | Default; uses border color on edges (causes visible seams) |
| `GL_CLAMP_TO_EDGE` | Ignores border texture; eliminates the black border |
| `GL_CLAMP_TO_BORDER` | Uses a specified border color |

`GL_CLAMP`, the default value for most cases, just uses the border color to set the texture on the border of the square. **`GL_CLAMP_TO_EDGE`** will be a good choice to ignore the texture on the border and erase the black border of the box.

Until now you should set up the skybox using the 5 sky images.

---

### 2.2 Water wave

Though we build up the sky, the ground will still be the pure color you choose. Another image (128×128 resolution) is used to create a water wave effect.

**Creating a static wave:**

If you try to map the image to the bottom of the box with texture coordinates ranging from 0.0 to 1.0, the wave will look very big and not clear since the resolution is low. A suggested approach is to set the wrapping mode of this texture to `GL_REPEAT` and give a coordinate larger than 1.0 where you would assign 1.0 originally. That way the texture will repeat itself, and the number of repetitions is decided by the maximum value you set for the texture.

**Animating the wave:**

Instead of giving a fixed coordinate to the vertex of the bottom face, make it changeable. For example, if you want to move the wave in the x-direction, the x texture coordinate should be incremented by a value (say *waveshift*) every time the frame is refreshed.

**Controlling wave speed:**

The flowing speed is related to the frame refreshing speed. For example, on Windows, `GetTickCount()` tells you how much time elapsed between adjacent frames, and you can use this value to decide the flow speed. Similar system calls exist on other platforms to ensure your water wave moves at the same speed across different machines.

**Sky reflection:**

To make the sea more realistic, we need a reflection of the sky. This is done by reversing the skybox and redrawing it. Since the reflected image is below the water wave, it won't be visible if depth testing is enabled — so mask the depth buffer before rendering the reflection (or simply disable depth test). **Blending** is also needed so that the reflection and the water are mixed. The color of the sea is determined by the blending factors you choose.

Until now, we have a beautiful scene with blue sky, white clouds, the sun, waving sea, and the reflection of the sky.

---

### 2.3 Terrain Model

The terrain model uses three images:

| File | Purpose |
|---|---|
| `heightmap.bmp` | Stores height information of the terrain (grayscale) |
| `terrain-texture3.bmp` | Color texture (mountains, grass, etc.) |
| `detail.bmp` | Adds fine detail to the terrain texture |

#### 2.3.1 Loading height information

`heightmap.bmp` is a grayscale image where every pixel value represents the terrain height. You can load it as an array or a matrix (e.g. using the SOIL library).

Having the height information, we can get a point (x, y, z) in 3D coordinate space from a position (i, j) in the image and its height value h:

```
(x, y, z) = (i, j, h)   // with scaling as needed
```

With these points, render them as triangles or quads by connecting adjacent points. No explicit terrain model storage is needed since the height data is organized in row/column-major order and can be accessed efficiently by index.

#### 2.3.2 Texture mapping

Since the terrain is rendered from height information in a regular grid, applying the color texture is straightforward. Texture coordinates for adjacent pixels have a constant increment of `1.0 / width` (or `1.0 / height`) of the grayscale image.

For the detail texture, **multi-texturing** is used:

- **1st texture** (`GL_TEXTURE0`): color texture
- **2nd texture** (`GL_TEXTURE1`): detail texture

Regarding texture environment modes:

- `GL_DECAL` and `GL_REPLACE` are not appropriate here.
- `GL_ADD` and `GL_MODULATE` (default) can produce weird colors due to clamping.
- **`GL_ADD_SIGNED`** is recommended as the best choice for combining the two textures naturally.

#### 2.3.3 Put it in the sea

When placing the terrain in the water, you may want to cut off the border ring of "water" around the terrain. Two approaches:

1. Use a **clip plane** to cut it out.
2. Set the blending function carefully when rendering the sky reflection and water wave so that the color of terrain parts below the water is excluded from blending.

A reflection of the terrain is also needed, using a procedure similar to the skybox reflection.

---

### 2.4 Interaction

A good wandering demo should have smooth interaction. Key points:

- All model-view matrix operations in OpenGL multiply a single matrix to the right of the current matrix, meaning objects have these operations applied in **reverse order**. Maintain the model-view matrix carefully.
- **Smooth movement**: When moving forward, speed up gradually (and stabilize at a certain speed) rather than jumping positions instantly; when stopping, decelerate gradually rather than stopping immediately. This requires careful control of movement speed.

---

## 3 Bonus

Any extra features can be considered for extra credit. The difficulty and quality of the effect will determine how much extra credit is awarded. Please describe the features you implement in the final report in detail.
