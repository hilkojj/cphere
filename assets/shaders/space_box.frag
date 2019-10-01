#version 300 es
precision mediump float;

out vec4 color;

in vec3 v_texCoords;

uniform samplerCube cubemap;
uniform float zoomedIn;

void main()
{
    float sky = min(1., max(0., (zoomedIn - .3) * 2.));
    color = vec4(.4, .6, .9, 1.) * sky + texture(cubemap, v_texCoords) * (1. - sky);
}
