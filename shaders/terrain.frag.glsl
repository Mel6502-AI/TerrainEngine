#version 330 core

in float vHeight;
in vec2 vTexCoord;
in vec2 vDetailCoord;
in vec3 vNormal;

uniform float uMinHeight;
uniform float uMaxHeight;
uniform sampler2D uColorTex;
uniform sampler2D uDetailTex;
uniform vec3 uLightDir;   // direction toward the sun (normalized in C++)

out vec4 color;

void main() {
    vec3 baseColor   = texture(uColorTex, vTexCoord).rgb;
    vec3 detailColor = texture(uDetailTex, vDetailCoord).rgb;
    vec3 combined = clamp(baseColor + detailColor - 0.5, 0.0, 1.0);

    // Directional (sun) lighting: ambient + Lambertian diffuse.
    vec3 N = normalize(vNormal);
    float diff = max(dot(N, normalize(uLightDir)), 0.0);
    float light = 0.25 + 0.95 * diff;   // lower ambient floor -> more visible sun shading

    color = vec4(combined * light, 1.0);
}