#version 430 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;

uniform mat4 MVP;
uniform vec3 sunDir;
uniform float camDist;

out vec3 v_color;
out float v_alpha;

float clamp(float x)
{
    return min(1, max(0, x));
}

void main()
{
    gl_Position = MVP * vec4(a_pos, 1);

    float camDot = 1 - dot(a_normal, vec3(0, 1, 0));
    float edge = camDot * 10 - (camDist / 80);
    edge = clamp(edge);
    v_alpha = clamp(camDot * 4.5 * max(.3, clamp(1 - camDist / 800))) - edge;


    float sunDot = dot(a_normal, sunDir);
    v_alpha *= clamp(sunDot * 2) + .5;

    float orangeDot = dot(a_normal, vec3(sunDir.x, 0, sunDir.z));

    // v_alpha = 1;

    float orange = clamp(orangeDot * 10 - 4.5);

    // v_color = vec3(orange);

    v_color = vec3(.6, .75, 1) * (1 - orange) + vec3(1, .9, .8) * orange;
}
