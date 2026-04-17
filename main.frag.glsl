#version 330 core

in vec3 Color;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int faceType; // 0 = colored, 1 = texture1 (SkyBox0), 2 = texture2 (SkyBox3)
uniform int texRotY; // Y rotation in degrees (0, 90, 180, 270)

void main()
{
    vec2 adjustedTex = TexCoords;

    // Apply Y rotation only for textured blue face (SkyBox3)
    if (faceType == 2) {
        if (texRotY == 90) {
            adjustedTex = vec2(1.0f - TexCoords.y, TexCoords.x);
        } else if (texRotY == 180) {
            adjustedTex = vec2(1.0f - TexCoords.x, 1.0f - TexCoords.y);
        } else if (texRotY == 270) {
            adjustedTex = vec2(TexCoords.y, 1.0f - TexCoords.x);
        }
    }

    if (faceType == 1) {
        color = texture(texture1, TexCoords);
    } else if (faceType == 2) {
        color = texture(texture2, adjustedTex);
    } else {
        color = vec4(Color, 1.0f);
    }
}
