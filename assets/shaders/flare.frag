#version 300 es
precision mediump float;
precision mediump sampler2DArray;

out vec4 color;

in vec2 v_texCoords;

uniform sampler2DArray textures;
uniform vec4 flareColor;
uniform int layer;

void main()
{
    color = flareColor * texture(textures, vec3(v_texCoords, layer)) * flareColor.a;
}
