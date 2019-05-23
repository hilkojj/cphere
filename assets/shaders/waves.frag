#version 330 core

in float v_wavefront, v_opacity;

out vec4 color;

void main()
{
    float a = v_opacity;
    a *= min(min(v_wavefront / .7, (1 - v_wavefront) / .2), 1);
    color = vec4(1, 1, 1, 1 - a);
}

