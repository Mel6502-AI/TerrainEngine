#include <string>
#include <iostream>
#include <map>
#include <sstream>
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
#include <ft2build.h>
#include FT_FREETYPE_H

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
GLuint loadTexture(const char * path, GLboolean alpha = false);
void drawSkyboxFaces(const Shader& shader,
                     GLuint skybox0Tex, GLuint skybox1Tex,
                     GLuint skybox2Tex, GLuint skybox3Tex,
                     GLuint skybox4Tex);

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

struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};
std::map<GLchar, Character> Characters;
GLuint textVAO, textVBO;
GLuint waterVAO, waterVBO;
GLuint terrainVAO, terrainVBO, terrainEBO;
GLuint terrainColorTexID, terrainDetailTexID;
GLsizei terrainIndexCount;
GLint locClipPlane = -1;

void drawSkyboxFaces(const Shader& shader,
                     GLuint skybox0Tex, GLuint skybox1Tex,
                     GLuint skybox2Tex, GLuint skybox3Tex,
                     GLuint skybox4Tex) {
    // Back face - SkyBox0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skybox0Tex);
    glUniform1i(glGetUniformLocation(shader.Program, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader.Program, "faceType"), 1);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotX"), 0);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotY"), 0);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotZ"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Front face - green - SkyBox2 (180Z flip)
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, skybox2Tex);
    glUniform1i(glGetUniformLocation(shader.Program, "texture4"), 3);
    glUniform1i(glGetUniformLocation(shader.Program, "faceType"), 4);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotX"), 180);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotY"), 0);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotZ"), 180);
    glDrawArrays(GL_TRIANGLES, 6, 6);

    // Left face - SkyBox3 (90Y + 180X)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, skybox3Tex);
    glUniform1i(glGetUniformLocation(shader.Program, "texture2"), 1);
    glUniform1i(glGetUniformLocation(shader.Program, "faceType"), 2);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotX"), 180);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotY"), 90);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotZ"), 0);
    glDrawArrays(GL_TRIANGLES, 12, 6);

    // Right face - yellow - SkyBox1 (90X flip)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skybox1Tex);
    glUniform1i(glGetUniformLocation(shader.Program, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader.Program, "faceType"), 1);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotX"), 90);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotY"), 0);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotZ"), 0);
    glDrawArrays(GL_TRIANGLES, 18, 6);

    // Top face - SkyBox4 (180Z flip)
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, skybox4Tex);
    glUniform1i(glGetUniformLocation(shader.Program, "texture3"), 2);
    glUniform1i(glGetUniformLocation(shader.Program, "faceType"), 3);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotX"), 180);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotY"), 180);
    glUniform1i(glGetUniformLocation(shader.Program, "texRotZ"), 180);
    glDrawArrays(GL_TRIANGLES, 30, 6);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Cube", NULL, NULL);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string exePath = "/Users/melvyn/computer_graphics/Sandbox_terrainEngine/";
    Shader shader((exePath + "main.vert.glsl").c_str(), (exePath + "main.frag.glsl").c_str());
    Shader textShader((exePath + "shaders/text.vert.glsl").c_str(), (exePath + "shaders/text.frag.glsl").c_str());
    Shader terrainShader((exePath + "terrain.vert.glsl").c_str(), (exePath + "terrain.frag.glsl").c_str());

    // Text projection (orthographic)
    glm::mat4 textProjection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), 0.0f, static_cast<GLfloat>(HEIGHT));

    // FreeType init
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    FT_Face face;
    if (FT_New_Face(ft, (exePath + "shaders/arial.ttf").c_str(), 0, &face))
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
    FT_Set_Pixel_Sizes(face, 0, 24);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character ch = { texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), GLuint(face->glyph->advance.x) };
        Characters.insert(std::pair<GLchar, Character>(c, ch));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Text VAO/VBO
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindVertexArray(0);

    // Load SkyBox textures
    GLuint skybox0Texture = loadTexture("/Users/melvyn/computer_graphics/TerrainEngine/data/SkyBox/SkyBox0.bmp", false);
    GLuint skybox1Texture = loadTexture("/Users/melvyn/computer_graphics/TerrainEngine/data/SkyBox/SkyBox1.bmp", false);
    GLuint skybox2Texture = loadTexture("/Users/melvyn/computer_graphics/TerrainEngine/data/SkyBox/SkyBox2.bmp", false);
    GLuint skybox3Texture = loadTexture("/Users/melvyn/computer_graphics/TerrainEngine/data/SkyBox/SkyBox3.bmp", false);
    GLuint skybox4Texture = loadTexture("/Users/melvyn/computer_graphics/TerrainEngine/data/SkyBox/SkyBox4.bmp", false);
    GLuint skybox5Texture = loadTexture("/Users/melvyn/computer_graphics/Sandbox_terrainEngine/data/SkyBox/SkyBox5.bmp", false);
    glBindTexture(GL_TEXTURE_2D, skybox5Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Load heightmap
    std::string heightmapPath = exePath + "data/heightmap.bmp";
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
    std::string colorTexPath = exePath + "data/terrain-texture3.bmp";
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
    std::string detailTexPath = exePath + "data/detail.bmp";
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
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        drawSkyboxFaces(shader, skybox0Texture, skybox1Texture,
                        skybox2Texture, skybox3Texture, skybox4Texture);

        // === 2. NORMAL SKYBOX (above-water world) ===
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(SCALE*SKYBOX_X, SCALE*SKYBOX_Y_POS, SCALE*SKYBOX_Z));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        drawSkyboxFaces(shader, skybox0Texture, skybox1Texture,
                        skybox2Texture, skybox3Texture, skybox4Texture);

        // === 3. REFLECTION PASS (flipped skybox below water at y=-80 to y=-30) ===
        glDepthMask(GL_FALSE);              // don't write depth
        glDisable(GL_DEPTH_TEST);           // don't test depth
        glDisable(GL_BLEND);               // opaque
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, SCALE*(SKYBOX_Y_NEG), 0.0f));  // shift so reflection top touches water at y=-100
        model = glm::scale(model, glm::vec3(SCALE*SKYBOX_X, SCALE*(SKYBOX_Y_NEG), SCALE*SKYBOX_Z));   // Y-flip
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        drawSkyboxFaces(shader, skybox0Texture, skybox1Texture,
                        skybox2Texture, skybox3Texture, skybox4Texture);

        // === TERRAIN PASS ===
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        terrainShader.Use();

        glm::mat4 terrainModel(1.0f);
        terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -0.5f * SCALE * SKYBOX_Y_POS - (-27.0f) - TERRAIN_Y_OFFSET, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(terrainModel));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(terrainShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(terrainShader.Program, "uDetailTiling"), DETAIL_TILING);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainColorTexID);
        glUniform1i(glGetUniformLocation(terrainShader.Program, "uColorTex"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainDetailTexID);
        glUniform1i(glGetUniformLocation(terrainShader.Program, "uDetailTex"), 1);

        glBindVertexArray(terrainVAO);
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

        glDepthMask(GL_TRUE);                // restore before text pass
        glBindVertexArray(0);

        // Draw HUD text - XYZ position (white, top left, smaller font)
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        textShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(textShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

        std::stringstream ss;
        ss.precision(2);
        ss << std::fixed << "X: " << camera.Position.x << "  Y: " << camera.Position.y << "  Z: " << camera.Position.z;
        RenderText(textShader, ss.str(), 10.0f, HEIGHT - 30.0f, 0.9f, glm::vec3(1.0f, 1.0f, 1.0f));

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
    shader.Use();
    glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos,     ypos,       0.0, 0.0 },
            { xpos + w, ypos,       1.0, 0.0 },
            { xpos,     ypos + h,   0.0, 1.0 },
            { xpos + w, ypos,       1.0, 0.0 },
            { xpos + w, ypos + h,   1.0, 1.0 }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Do_Movement()
{
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_LEFT_SHIFT])
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (keys[GLFW_KEY_SPACE])
        camera.ProcessKeyboard(UP, deltaTime);
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
