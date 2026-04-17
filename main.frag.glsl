#version 330 core

in vec3 Color;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform int faceType; // 0 = colored, 1 = SkyBox1, 2 = SkyBox3, 3 = SkyBox4, 4 = SkyBox2, 5 = water
uniform int texRotY;  // Y rotation in degrees (0, 90, 180, 270) - for faces 2 and 3
uniform int texRotX;  // X rotation in degrees (0, 180) - for face 2
uniform int texRotZ;  // Z rotation in degrees (0, 90, 180, 270) - for face 3

void main()
{
    vec2 adjustedTex = TexCoords;

    // Blue face - SkyBox3 with Y+X rotation
    if (faceType == 2) {
        if (texRotY == 90) {
            adjustedTex = vec2(1.0f - TexCoords.y, TexCoords.x);
        } else if (texRotY == 180) {
            adjustedTex = vec2(1.0f - TexCoords.x, 1.0f - TexCoords.y);
        } else if (texRotY == 270) {
            adjustedTex = vec2(TexCoords.y, 1.0f - TexCoords.x);
        }

        if (texRotX == 180) {
            adjustedTex.y = 1.0f - adjustedTex.y;
        }
    } else if (faceType == 3) {
        // Cyan face - SkyBox4 with Y and Z rotation
        if (texRotY == 180) {
            adjustedTex = vec2(1.0f - TexCoords.x, 1.0f - TexCoords.y);
        }

        if (texRotX == 180) {
            adjustedTex.y = 1.0f - adjustedTex.y;
        }

        if (texRotZ == 90) {
            adjustedTex = vec2(TexCoords.y, 1.0f - TexCoords.x);
        } else if (texRotZ == 180) {
            adjustedTex = vec2(1.0f - adjustedTex.x, 1.0f - adjustedTex.y);
        } else if (texRotZ == 270) {
            adjustedTex = vec2(1.0f - TexCoords.y, TexCoords.x);
        }
    }

    if (faceType == 1) {
        if (texRotX == 90) {
            adjustedTex = vec2(TexCoords.y, 1.0f - TexCoords.x);
        } else if (texRotX == 180) {
            adjustedTex.y = 1.0f - TexCoords.y;
        }
        color = texture(texture1, adjustedTex);
    } else if (faceType == 2) {
        color = texture(texture2, adjustedTex);
    } else if (faceType == 3) {
        color = texture(texture3, adjustedTex);
    } else if (faceType == 4) {
        if (texRotZ == 180) {
            adjustedTex = vec2(1.0f - TexCoords.x, 1.0f - TexCoords.y);
        }
        if (texRotX == 180) {
            adjustedTex.y = 1.0f - adjustedTex.y;
        }
        color = texture(texture4, adjustedTex);
    } else if (faceType == 5) {
        color = texture(texture5, TexCoords);
    } else {
        color = vec4(Color, 1.0f);
    }
}
