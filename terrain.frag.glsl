#version 330 core

in float vHeight;
in vec2 vTexCoord;
in vec2 vDetailCoord;

uniform float uMinHeight;
uniform float uMaxHeight;
uniform sampler2D uColorTex;
uniform sampler2D uDetailTex;

out vec4 color;

void main() {
    vec3 baseColor   = texture(uColorTex, vTexCoord).rgb;
    vec3 detailColor = texture(uDetailTex, vDetailCoord).rgb;
    vec3 combined = clamp(baseColor + detailColor - 0.5, 0.0, 1.0);
    color = vec4(combined, 1.0);
}