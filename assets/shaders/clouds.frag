#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_texCoords, v_pos;
in float progress, opacity;

uniform sampler2D noiseTex;

void main()
{
    float texA = textureLod(noiseTex, v_texCoords, 6. - 6. * opacity).r;
    color = vec4(1, 1, 1, texA * opacity * .4);
    color.a *= 1. - length(v_pos);
}
