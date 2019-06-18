#version 300 es
precision mediump float;

out vec4 color;

in vec3 v_texCoords;

uniform samplerCube cubemap;

void main()
{
    color = texture(cubemap, v_texCoords);
}
