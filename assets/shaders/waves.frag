#version 330 core

in float v_wavefront, v_opacity;

out vec4 color;

void main()
{
    color = vec4(1, 1, 1, v_opacity * v_wavefront);
}

