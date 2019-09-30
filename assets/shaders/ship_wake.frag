#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;

void main()
{
    float x = v_uv.x;

    float a = max(0., 1. - pow(x * 5. - 1., 2.));

    a += max(0., 1. - pow(x * 5. - 4., 2.));



    // color = vec4(a * (1. - v_uv.y));

    a *= 1. - v_uv.y;

    color = vec4(1, 1, 1, 1. - a);
}
