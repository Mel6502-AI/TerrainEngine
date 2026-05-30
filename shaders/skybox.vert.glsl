#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 TexDir;

void main()
{
    // Sample the cubemap by direction. mat3(model) carries the per-pass rotation/scale,
    // so a reflection pass that scales by (1,-1,1) yields a Y-flipped (mirrored) sky.
    TexDir = mat3(model) * aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
