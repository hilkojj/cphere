#version 300 es
precision mediump float;
precision mediump int;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nor;
layout(location = 2) in vec3 a_tan;
layout(location = 3) in vec2 a_uv;

layout(location = 4) in mat4 transform;
layout(location = 8) in float random;

uniform mat4 view;
uniform vec3 sunDir;
uniform float time;

uniform int justPlacedId;
uniform vec4 timeSincePlacing;

out vec2 v_uv;
out float light, y;
flat out float v_random;
flat out int instanceId;

float circleOut(float x)
{
    x--;
    return sqrt(1. - x * x);
}


void main() {

    vec3 pos = a_pos;
    y = pos.y;
    instanceId = gl_InstanceID;
    if (instanceId >= justPlacedId)
    {
        // placing animation:

        float animTime = circleOut(clamp(timeSincePlacing[instanceId - justPlacedId], 0., .5) * 2.);

        pos.y -= (1. - animTime) * 8.;

        animTime = circleOut(clamp(timeSincePlacing[instanceId - justPlacedId], 0., 1.));

        pos.xz *= animTime;
    }


    float waveEffectX = clamp(pos.y * .3 - .1, 0., 1.);
    waveEffectX *= waveEffectX;

    float waveTime = time * clamp(random, .1, 1.) * 2.;

    float waveIntensity = clamp(fract(sin(random)*8.560), .1, .3) * .2;

    pos.x += sin(waveTime) * waveEffectX * waveIntensity;

    float waveEffectY = -pos.x;

    pos.y += sin(waveTime) * waveEffectX * waveEffectY * waveIntensity;

    gl_Position = (view * transform) * vec4(pos, 1);
    light = dot((transform * vec4(a_nor, 0)).xyz, sunDir) * .3 + .7;

    v_uv = a_uv;
    v_random = random;
}
