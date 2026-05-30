#include <string>
#include <iostream>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLuint loadTexture(const char * path, GLboolean alpha = false);
GLuint loadCubemapCross(const std::string& path);
void drawSkyCube(const Shader& sky, GLuint cubemap, GLuint cubeVAO,
                 const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

GLuint WIDTH = 800, HEIGHT = 600;
#define WATER_SPEED_X 0.1f
#define WATER_SPEED_Y 0.015f
#define SCALE 100.0f
#define SKYBOX_X 60.0f
#define SKYBOX_Y_POS 30.0f
#define SKYBOX_Y_NEG -30.0f
#define SKYBOX_Z 70.0f
#define FADE_START (12.0f * SCALE)
#define FADE_END (30.0f * SCALE)
#define XZ_SCALE      0.2f
#define Y_SCALE       20.0f
#define Y_OFFSET     -27.0f
#define DETAIL_TILING 32.0f
#define SPAWN_HEIGHT_ABOVE_WATER 20.0f
#define TERRAIN_Y_OFFSET 5.0f

Camera camera(glm::vec3(0.0f, -0.5f * SCALE * SKYBOX_Y_POS + SPAWN_HEIGHT_ABOVE_WATER, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint waterVAO, waterVBO;
GLuint terrainVAO, terrainVBO, terrainEBO;
GLuint terrainColorTexID, terrainDetailTexID;
GLsizei terrainIndexCount;
GLint locClipPlane = -1;

// Loads a horizontal-cross cubemap PNG (4x3 grid of square cells) into a GL cubemap.
// Cell layout (col,row), 0-indexed:  +Y at (1,0); -X,+Z,+X,-Z across row 1; -Y at (1,2).
// Faces are sliced straight out of the single decoded image via GL_UNPACK_ROW_LENGTH,
// so no external image tooling is needed.
GLuint loadCubemapCross(const std::string& path) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    int W, H, n;
    stbi_set_flip_vertically_on_load(false);   // cubemap faces are stored top-down
    unsigned char* img = stbi_load(path.c_str(), &W, &H, &n, 3);
    if (!img) {
        std::cout << "Failed to load cubemap: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
        return tex;
    }
    const int cw = W / 4, ch = H / 3;   // cell size (e.g. 512x512)
    struct Cell { GLenum face; int col, row; };
    const Cell cells[6] = {
        { GL_TEXTURE_CUBE_MAP_POSITIVE_X, 2, 1 },
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 1 },
        { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1, 0 },
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 1, 2 },
        { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 1, 1 },
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 3, 1 },
    };
    glPixelStorei(GL_UNPACK_ROW_LENGTH, W);     // let each face read a sub-rectangle
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (const Cell& c : cells) {
        const unsigned char* p = img + (c.row * ch * W + c.col * cw) * 3;
        glTexImage2D(c.face, 0, GL_RGB, cw, ch, 0, GL_RGB, GL_UNSIGNED_BYTE, p);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    stbi_image_free(img);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return tex;
}

// Draws the skybox cube with a cubemap. The cube keeps its existing finite size and
// per-pass model matrix, so the reflection passes (which scale by -1 in Y) sample the
// cubemap with a Y-flipped direction and produce a mirrored sky for free.
void drawSkyCube(const Shader& sky, GLuint cubemap, GLuint cubeVAO,
                 const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    sky.Use();
    glUniformMatrix4fv(glGetUniformLocation(sky.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(sky.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(sky.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glUniform1i(glGetUniformLocation(sky.Program, "skybox"), 0);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);   // request a 4x MSAA framebuffer (anti-aliased edges)
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Terrain Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);   // enable 4x MSAA requested via GLFW_SAMPLES
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // PROJECT_ROOT is injected by CMake (target_compile_definitions); assets resolve
    // relative to the source tree, so the binary runs from any working directory.
    const std::string ROOT = PROJECT_ROOT;
    Shader shader((ROOT + "/shaders/main.vert.glsl").c_str(), (ROOT + "/shaders/main.frag.glsl").c_str());
    Shader terrainShader((ROOT + "/shaders/terrain.vert.glsl").c_str(), (ROOT + "/shaders/terrain.frag.glsl").c_str());
    Shader skyboxShader((ROOT + "/shaders/skybox.vert.glsl").c_str(), (ROOT + "/shaders/skybox.frag.glsl").c_str());

    // Load SkyBox textures
    GLuint skyCubemap = loadCubemapCross(ROOT + "/data/SkyBox/cubemap.png");
    GLuint skybox5Texture = loadTexture((ROOT + "/data/SkyBox/SkyBox5.bmp").c_str(), false);
    glBindTexture(GL_TEXTURE_2D, skybox5Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Load heightmap
    std::string heightmapPath = ROOT + "/data/heightmap.bmp";
    std::cout << "Loading heightmap: " << heightmapPath << std::endl;
    int hmW, hmH, hmChannels;
    unsigned char* hmImage = stbi_load(heightmapPath.c_str(), &hmW, &hmH, &hmChannels, 1);
    if (!hmImage) {
        std::cout << "Failed to load heightmap: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    std::cout << "Heightmap loaded: " << hmW << " x " << hmH << std::endl;

    // Build terrain mesh
    std::vector<float> terrainVerts;
    std::vector<GLuint> terrainIndices;

    for (int j = 0; j < hmH; ++j) {
        for (int i = 0; i < hmW; ++i) {
            float x = (i - hmW / 2.0f) * XZ_SCALE;
            float y = (hmImage[j * hmW + i] / 255.0f) * Y_SCALE + Y_OFFSET;
            float z = (j - hmH / 2.0f) * XZ_SCALE;
            float u = i / (hmW - 1.0f);
            float v = j / (hmH - 1.0f);
            terrainVerts.insert(terrainVerts.end(), {x, y, z, u, v});
        }
    }
    for (int j = 0; j < hmH - 1; ++j) {
        for (int i = 0; i < hmW - 1; ++i) {
            GLuint i00 = j * hmW + i;
            GLuint i10 = j * hmW + (i + 1);
            GLuint i01 = (j + 1) * hmW + i;
            GLuint i11 = (j + 1) * hmW + (i + 1);
            terrainIndices.insert(terrainIndices.end(), {
                i00, i10, i11,
                i00, i11, i01
            });
        }
    }
    stbi_image_free(hmImage);

    terrainIndexCount = terrainIndices.size();
    std::cout << "Terrain: " << terrainVerts.size() / 5 << " vertices, " << terrainIndexCount / 3 << " triangles" << std::endl;

    locClipPlane = glGetUniformLocation(terrainShader.Program, "uClipPlane");

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);
    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, terrainVerts.size() * sizeof(float), terrainVerts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainIndices.size() * sizeof(GLuint), terrainIndices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // Load color texture
    std::string colorTexPath = ROOT + "/data/terrain-texture3.bmp";
    std::cout << "Loading color texture: " << colorTexPath << std::endl;
    int ctW, ctH, ctChannels;
    unsigned char* ctImage = stbi_load(colorTexPath.c_str(), &ctW, &ctH, &ctChannels, 3);
    if (!ctImage) {
        std::cout << "Failed to load color texture: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    std::cout << "Color texture loaded: " << ctW << " x " << ctH << std::endl;
    glGenTextures(1, &terrainColorTexID);
    glBindTexture(GL_TEXTURE_2D, terrainColorTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctW, ctH, 0, GL_RGB, GL_UNSIGNED_BYTE, ctImage);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(ctImage);

    // Load detail texture
    std::string detailTexPath = ROOT + "/data/detail.bmp";
    std::cout << "Loading detail texture: " << detailTexPath << std::endl;
    int dtW, dtH, dtChannels;
    unsigned char* dtImage = stbi_load(detailTexPath.c_str(), &dtW, &dtH, &dtChannels, 3);
    if (!dtImage) {
        std::cout << "Failed to load detail texture: " << stbi_failure_reason() << std::endl;
        return -1;
    }
    std::cout << "Detail texture loaded: " << dtW << " x " << dtH << std::endl;
    glGenTextures(1, &terrainDetailTexID);
    glBindTexture(GL_TEXTURE_2D, terrainDetailTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dtW, dtH, 0, GL_RGB, GL_UNSIGNED_BYTE, dtImage);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(dtImage);

    // Cube vertices
    GLfloat cubeVertices[] = {
        // Back face (SkyBox0) - indices 0-5
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        // Front face (green) - indices 6-11
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        // Left face (SkyBox3) - indices 12-17
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,

        // Right face (yellow) - indices 18-23
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f, 1.0f, 0.0f,

        // Bottom face (magenta) - indices 24-29 - water texture with tiling
        -0.5f, -0.5f, -0.5f,  0.0f,  8.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  8.0f,  8.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  8.0f,  0.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  8.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  8.0f,  1.0f, 0.0f, 1.0f,

        // Top face (SkyBox4) - indices 30-35
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f
    };

    // Setup cube VAO
    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Water quad VAO/VBO - separate from skybox cube
    GLfloat waterVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 1.0f
    };
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        static struct timespec lastTime = {0, 0};
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        float dt = 0.0f;
        if (lastTime.tv_sec != 0 || lastTime.tv_nsec != 0) {
            dt = (now.tv_sec  - lastTime.tv_sec)
               + (now.tv_nsec - lastTime.tv_nsec) / 1e9f;
        }
        lastTime = now;
        deltaTime = dt;

        glfwPollEvents();
        Do_Movement();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        glm::mat4 model(1);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 1000000.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shader.Program, "uCameraPos"), 1, glm::value_ptr(camera.Position));

        // === 1. REFLECTION PASS (flipped skybox drawn below water, depth-tested against scene) ===
        glDepthMask(GL_FALSE);              // don't write depth
        glEnable(GL_DEPTH_TEST);             // test depth — reflection occludes behind terrain
        glDepthFunc(GL_LEQUAL);             // standard depth test
        glDisable(GL_BLEND);               // reflection itself is opaque
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(SCALE*SKYBOX_X, SCALE*(SKYBOX_Y_NEG), SCALE*SKYBOX_Z));  // Y-flip
        drawSkyCube(skyboxShader, skyCubemap, cubeVAO, model, view, projection);

        // === 2. NORMAL SKYBOX (above-water world) ===
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(SCALE*SKYBOX_X, SCALE*SKYBOX_Y_POS, SCALE*SKYBOX_Z));
        drawSkyCube(skyboxShader, skyCubemap, cubeVAO, model, view, projection);

        // === 3. REFLECTION PASS (flipped skybox below water at y=-80 to y=-30) ===
        glDepthMask(GL_FALSE);              // don't write depth
        glDisable(GL_DEPTH_TEST);           // don't test depth
        glDisable(GL_BLEND);               // opaque
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, SCALE*(SKYBOX_Y_NEG), 0.0f));  // shift so reflection top touches water at y=-100
        model = glm::scale(model, glm::vec3(SCALE*SKYBOX_X, SCALE*(SKYBOX_Y_NEG), SCALE*SKYBOX_Z));   // Y-flip
        drawSkyCube(skyboxShader, skyCubemap, cubeVAO, model, view, projection);

        // === TERRAIN PASS (shared setup for all terrain draws) ===
        terrainShader.Use();
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glBindVertexArray(terrainVAO);

        // Compute terrain model matrix (used by both reflection and above-water passes)
        glm::mat4 terrainModel(1.0f);
        terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -0.5f * SCALE * SKYBOX_Y_POS - (-27.0f) - TERRAIN_Y_OFFSET, 0.0f));

        // Shared uniforms (view/proj/detailTiling/textures — same for all terrain draws)
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(terrainShader.Program, "uDetailTiling"), DETAIL_TILING);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainColorTexID);
        glUniform1i(glGetUniformLocation(terrainShader.Program, "uColorTex"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainDetailTexID);
        glUniform1i(glGetUniformLocation(terrainShader.Program, "uDetailTex"), 1);

        // === TERRAIN REFLECTION PASS (mirrored terrain below water) ===
        // Mirror transform: T(0, 2*WATER_LEVEL, 0) * S(1,-1,1).
        // With WATER_LEVEL=-1500: T(0, -3000, 0) * S(1,-1,1) maps (x,y,z) to (x, -3000-y, z).
        glm::mat4 mirrorMat(1.0f);
        mirrorMat = glm::translate(mirrorMat, glm::vec3(0.0f, 2.0f * (-1500.0f), 0.0f));
        mirrorMat = glm::scale(mirrorMat, glm::vec3(1.0f, -1.0f, 1.0f));
        glm::mat4 reflectedTerrainModel = mirrorMat * terrainModel;
        glEnable(GL_CLIP_DISTANCE0);
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(reflectedTerrainModel));
        // Keep only y <= -1500 (below water surface): plane (0, -1, 0, -1500)
        glUniform4f(locClipPlane, 0.0f, -1.0f, 0.0f, -1500.0f);
        glFrontFace(GL_CW);   // scale(1,-1,1) flips winding
        glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
        glFrontFace(GL_CCW);  // restore default
        glDisable(GL_CLIP_DISTANCE0);

        // === TERRAIN PASS (above water) ===
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(terrainModel));
        glEnable(GL_CLIP_DISTANCE0);
        glUniform4f(locClipPlane, 0.0f, 1.0f, 0.0f, 1500.0f);  // clip at y >= -1500 (water surface world y)
        glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
        glDisable(GL_CLIP_DISTANCE0);
        glBindVertexArray(0);

        // Switch back to main shader for the water pass
        shader.Use();

        // === 4. WATER (blended over reflection) ===
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);              // water is transparent → don't block later transparent objects
        glBindVertexArray(waterVAO);
        // Match the skybox scale so water sits at the same Y as the reflection's "seam"
        model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(SCALE*SKYBOX_X, SCALE*SKYBOX_Y_POS, SCALE*SKYBOX_Z));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, skybox5Texture);
        glUniform1i(glGetUniformLocation(shader.Program, "texture5"), 4);
        glUniform1i(glGetUniformLocation(shader.Program, "faceType"), 5);
        glUniform1i(glGetUniformLocation(shader.Program, "texRotX"), 0);
        glUniform1i(glGetUniformLocation(shader.Program, "texRotY"), 0);
        glUniform1i(glGetUniformLocation(shader.Program, "texRotZ"), 0);
        glUniform1f(glGetUniformLocation(shader.Program, "uWaterTiling"), 16.0f * (SCALE / 5.0f));
        glUniform1f(glGetUniformLocation(shader.Program, "uFadeStart"), FADE_START);
        glUniform1f(glGetUniformLocation(shader.Program, "uFadeEnd"), FADE_END);
        static float waveShift = 0.0f;
        waveShift += WATER_SPEED_X * dt;
        // if (waveShift > 64.0f) waveShift -= 64.0f;
        glUniform1f(glGetUniformLocation(shader.Program, "uWaveShift"), waveShift);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDepthMask(GL_TRUE);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void Do_Movement()
{
    // Build the desired movement direction from the held keys, then let the
    // camera smoothly accelerate toward it / decelerate when nothing is pressed.
    glm::vec3 wish(0.0f);
    glm::vec3 flatFront(camera.Front.x, 0.0f, camera.Front.z);  // ignore pitch for W/S
    if (glm::length(flatFront) > 0.0f)
        flatFront = glm::normalize(flatFront);

    if (keys[GLFW_KEY_W])          wish += flatFront;
    if (keys[GLFW_KEY_S])          wish -= flatFront;
    if (keys[GLFW_KEY_A])          wish -= camera.Right;
    if (keys[GLFW_KEY_D])          wish += camera.Right;
    if (keys[GLFW_KEY_SPACE])      wish += camera.WorldUp;
    if (keys[GLFW_KEY_LEFT_SHIFT]) wish -= camera.WorldUp;

    camera.ApplyMovement(wish, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

GLuint loadTexture(const char * path, GLboolean alpha)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* image = stbi_load(path, &width, &height, &nrComponents, 0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, alpha ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    return textureID;
}
