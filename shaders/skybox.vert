#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 vTexCoord;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vTexCoord     = aPos;
    // W component = 1.0 so translation is ignored in view matrix
    vec4 pos      = projection * view * vec4(aPos, 1.0);
    // Keep clip-space position for depth
    gl_Position   = pos.xyww;  // Forces skybox to far plane
}
