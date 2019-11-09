#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;
in float light, y;

uniform sampler2D buildingTexture;
uniform float time;
uniform bool blocked;

void main()
{
    color = texture(buildingTexture, v_uv);
    if (color.a <= .7) discard;

    color.rgb *= light;

    if (blocked)
    {
        color.rgb += vec3(.1);
        color.rgb *= vec3(2., .4, .4);
        return;
    }


    float white = pow(clamp(abs(sin(y * .2 + time * 2.)), .0, 1.), 10.);

    white += pow(clamp(abs(sin(y * .2 + time * -2. - 3.14)), .0, 1.), 10.);

    color.rgb *= 1. + white;
}
