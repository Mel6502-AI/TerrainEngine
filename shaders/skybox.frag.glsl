#version 330 core

in vec3 TexDir;
out vec4 color;

uniform samplerCube skybox;

void main()
{
    color = texture(skybox, normalize(TexDir));
}
