#version 330 core

in vec3 Color;
in vec2 TexCoords;
in float FaceId;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    if (FaceId > 0.5f && FaceId < 1.5f) {
        // Face 1 (left) uses texture2
        color = texture(texture2, TexCoords);
    } else if (FaceId > 1.5f) {
        // Faces 2+ use texture1
        color = texture(texture1, TexCoords);
    } else {
        // Face 0 (back) uses colored approach but texture1
        color = vec4(Color, 1.0f);
    }
}
