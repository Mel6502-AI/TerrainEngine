#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 uClipPlane;  // (x, y, z, d) — clip at dot(vec4(worldPos, 1.0), uClipPlane) >= 0

out float vHeight;
out vec2 vTexCoord;
out vec2 vDetailCoord;
out float gl_ClipDistance[1];

uniform float uDetailTiling;

void main() {
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = projection * view * worldPos;
    gl_ClipDistance[0] = dot(worldPos, uClipPlane);
    vHeight = position.y;
    vTexCoord = texCoord;
    vDetailCoord = texCoord * uDetailTiling;
}