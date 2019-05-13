#version 430 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;

uniform mat4 MVP;
uniform vec3 sunDir;
uniform float camDist;

out float v_alpha;

float clamp(float x)
{
    return min(1, max(0, x));
}

void main()
{
    gl_Position = MVP * vec4(a_pos, 1);

    float dot = 1 - dot(a_normal, vec3(0, 1, 0));

    // v_alpha = min(1, dot * 10);
    float edge = dot * 10 - (camDist / 80);
    edge = clamp(edge);

    v_alpha = clamp(dot * 4.5 * max(.3, clamp(1 - camDist / 800))) - edge;
}
