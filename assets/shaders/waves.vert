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
    float t = timer;
    if (a_wavefront < .5)
        t = max(0., timer * timer * .7);
    vec3 pos = a_pos0 * (1 - t) + a_pos1 * t;
    
    gl_Position = view * vec4(pos, 1);

    v_wavefront = a_wavefront;
    v_opacity = a_opacity * min(timer * 3, 1) * min(1, 2 - timer * 2);
}

