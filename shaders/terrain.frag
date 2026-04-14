#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

uniform sampler2D terrainTexture;
uniform vec3 lightDir;
uniform vec3 viewPos;

void main() {
    // Ambient
    vec3 ambient = 0.25 * vec3(1.0);

    // Diffuse
    vec3 norm    = normalize(vNormal);
    vec3 light   = normalize(lightDir);
    float diff   = max(dot(norm, light), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    // Specular (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 halfway  = normalize(light + viewDir);
    float spec    = pow(max(dot(norm, halfway), 0.0), 32.0);
    vec3 specular = 0.3 * spec * vec3(1.0);

    // Terrain texture
    vec3 texColor = texture(terrainTexture, vTexCoord).rgb;

    vec3 result = (ambient + diffuse + specular) * texColor;
    FragColor   = vec4(result, 1.0);
}
