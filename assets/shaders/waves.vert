#version 330 core

layout(location = 0) in vec3 a_pos0;
layout(location = 1) in vec3 a_pos1;
layout(location = 2) in float a_wavefront;
layout(location = 3) in float a_opacity;

out float v_wavefront;
out float v_opacity;

uniform mat4 view;
uniform float timer;

void main()
{
    vec3 pos = a_wavefront < .5 ? a_pos0 : a_pos0 * (1 - timer) + a_pos1 * timer;
    gl_Position = view * vec4(pos, 1);

    v_wavefront = a_wavefront;
    v_opacity = a_opacity;
}

