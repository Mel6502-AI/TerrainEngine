#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 vertexColor;
layout (location = 3) in float faceId;

out vec3 Color;
out vec2 TexCoords;
out float FaceId;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    Color = vertexColor;
    FaceId = faceId;

    // Apply texture coordinate adjustments per face
    vec2 tex = texCoord;
    if (faceId > 0.5f && faceId < 1.5f) {
        // Left face (id=1) - rotate 90 degrees
        tex = vec2(1.0f - texCoord.y, texCoord.x);
    }
    TexCoords = tex;
}
