#version 300 es
precision mediump float;

in float v_wavefront, v_opacity;

out vec4 color;

void main()
{
    float a = v_opacity;
    a *= min(min(v_wavefront / .8, (1 - v_wavefront) / .1), 1);
    color = vec4(1, 1, 1, 1 - a);
}

