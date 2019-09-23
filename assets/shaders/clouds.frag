#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_texCoords, v_pos;
in vec3 cloudColor;
in float opacity;

uniform sampler2D noiseTex;

void main()
{
    // discard;
    float texA = textureLod(noiseTex, v_texCoords, 6. - 6. * opacity).r;
    texA *= opacity;
    texA *= 1. - length(v_pos);

    color = vec4(cloudColor, texA);
}
