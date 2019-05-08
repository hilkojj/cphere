#version 330 core
out vec3 color;

in vec3 v_normal;
in vec3 v_tangent;
in vec2 v_texCoords;

uniform sampler2D waterNormals;

void main()
{
    color = texture(waterNormals, v_texCoords * 10).rgb;
}
