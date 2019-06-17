#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_texCoords;

uniform sampler2D sun, depthTex;

const float near = .1, far = 1000.;
uniform vec2 scrSize;
uniform float time;

float getDepth(vec2 screenCoords)
{
    if (screenCoords.x < 0. || screenCoords.x > 1. || screenCoords.y < 0. || screenCoords.y > 1.) return 0.;
    return 2. * near * far / (far + near - (2. * texture(depthTex, screenCoords).r - 1.) * (far - near));
}

vec2 rotateUV(vec2 uv, float rotation, float mid)
{
    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}

void main()
{
    vec2 screenCoords = gl_FragCoord.xy / scrSize;
    color = vec4(0);

    vec2 offset = v_texCoords * vec2(2) - vec2(1);
    float x = smoothstep(1., 0., length(offset));

    float sunA = texture(sun, rotateUV(v_texCoords, time * -.05, .5)).r + texture(sun, rotateUV(v_texCoords, time * .03, .5)).r;

    if (x == 0.) sunA = 0.;

    float rays = 1.;

    if (getDepth(screenCoords) > far - 10.)
    {
        color = vec4(vec3(1), sunA);
        rays = .5;
    }

    vec2 correctOffset = vec2(offset.x * (scrSize.y / scrSize.x), offset.y) * -.2;

    const int steps = 60;
    float depth = 0.;
    for (int i = 0; i < steps; i++)
    {
        depth += getDepth(screenCoords + correctOffset * (float(i) / float(steps))) > (far - 10.) ? 1. : 0.;
    }
    depth /= 0.0 + float(steps);

    color += vec4(vec3(1), depth * x) * rays;

    color.rgb *= x;
    color.rgb += (1. - x) * vec3(1, .5, .3);
}
