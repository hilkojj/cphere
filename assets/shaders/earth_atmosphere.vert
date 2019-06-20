#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;

uniform mat4 MVP;
uniform vec3 sunDir;
uniform float camDist;

out vec3 v_color;
out float v_alpha;

float clamp1(float x)
{
    return min(1., max(0., x));
}

void main()
{
    gl_Position = MVP * vec4(a_pos, 1);

    float camDot = 1. - dot(a_normal, vec3(0, 1, 0));
    float edge = camDot * 10. - (camDist / 150.);
    edge = clamp1(edge);
    float middle = camDot * 8. * max(.1, clamp1(1. - camDist / 500.));
    middle = clamp1(middle);
    v_alpha = middle - edge;

    float sunDot = dot(a_normal, sunDir);

    v_alpha *= clamp1(sunDot * 2.) + .5;

    float orangeDot = dot(a_normal, vec3(sunDir.x, 0, sunDir.z));
    float orange = clamp1(orangeDot * 10. - 4.5);

    v_color = vec3(.3, .5, 1) * edge + vec3(.6, .75, 1) * (1. - edge);
}
