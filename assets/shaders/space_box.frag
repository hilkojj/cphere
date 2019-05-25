#version 330 core
out vec3 color;

in vec3 v_texCoords;

uniform samplerCube cubemap;

void main()
{
    color = texture(cubemap, v_texCoords).rgb;
}
