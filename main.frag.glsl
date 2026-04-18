#version 330 core

#define SCALE 100.0f

in vec3 Color;
in vec2 TexCoords;
in vec3 WorldPos;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform int faceType; // 0 = colored, 1 = SkyBox1, 2 = SkyBox3, 3 = SkyBox4, 4 = SkyBox2, 5 = water
uniform float uWaveShift;
uniform int texRotY;  // Y rotation in degrees (0, 90, 180, 270) - for faces 2 and 3
uniform int texRotX;  // X rotation in degrees (0, 180) - for face 2
uniform int texRotZ;  // Z rotation in degrees (0, 90, 180, 270) - for face 3
uniform vec3 uCameraPos;
uniform float uWaterTiling;
uniform float uFadeStart;
uniform float uFadeEnd;

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
        // Alpha fade: increases water transparency near camera, fully opaque at distance
        const float nearAlpha = 0.55f;

        // Water: sample with scrolling UVs, output with alpha for blending
        vec2 uv = vec2(TexCoords.x * uWaterTiling + uWaveShift, TexCoords.y * uWaterTiling + 0.5 * uWaveShift);
        vec4 waterCol = texture(texture5, uv);
        // Tint toward a sea-blue and let reflection show through
        vec3 seaTint = vec3(0.25, 0.55, 0.75);
        waterCol.rgb = mix(waterCol.rgb, seaTint, 0.3);

        // Ramp alpha from nearAlpha (transparent near camera) to 1.0 (opaque at distance)
        float dist = length(WorldPos - uCameraPos);
        float fade = smoothstep(uFadeStart, uFadeEnd, dist);
        waterCol.a = mix(nearAlpha, 1.0f, fade);

        color = vec4(waterCol.rgb, waterCol.a);
    } else {
        color = vec4(Color, 1.0f);
    }
}
