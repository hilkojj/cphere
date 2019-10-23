#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;
in float light, v_random;

uniform sampler2D buildingTexture;

void main()
{
    color = texture(buildingTexture, v_uv);
    if (color.a <= .7) discard;
    color.a *= 3.;

    color.rgb *= light;

    if (color.g < color.r) return; // only change color of leaves

    float r = v_random * v_random * v_random;

    vec3 c = v_random > .5 ? vec3(1., 1.5, 1.4) : vec3(10., 1., 1.4);

    color.rgb *= c * r + vec3(1. - r);

    r *= .5;
    color.rgb = (vec3(color.r + color.g + color.b) / 3.) * r + color.rgb * (1. - r);
}
