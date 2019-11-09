#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nor;
layout(location = 2) in vec3 a_tan;
layout(location = 3) in vec2 a_uv;

layout(location = 4) in mat4 transform;
layout(location = 8) in float random;

uniform mat4 view;
uniform vec3 sunDir;
uniform float time;

out vec2 v_uv;
out float light, v_random, y;

flat out int instanceId;

uniform int justPlacedId;
uniform vec4 timeSincePlacing;

#define PI 3.14159265

float circleOut(float x)
{
    x--;
    return sqrt(1. - x * x);
}


void main() {

    vec3 pos = a_pos;
    instanceId = gl_InstanceID;

    if (instanceId >= justPlacedId)
    {
        // placing animation:
        float animTime = clamp(timeSincePlacing[instanceId - justPlacedId], 0., .5) * 2.;

        float scale = sin(circleOut(animTime) * 1. * PI - PI) + 1.;

        pos.y *= scale * .3 + .7;
        pos.xz *= 1.1 - scale * .1;

        //        if (10. - y > animTime * 10.)
        //        discard;
    }

    gl_Position = (view * transform) * vec4(pos, 1);
    y = a_pos.y;
    light = dot((transform * vec4(a_nor, 0)).xyz, sunDir) * .3 + .7;

    v_uv = a_uv;
    v_random = random;
}
