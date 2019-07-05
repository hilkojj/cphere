#version 300 es
#define PI 3.1415926535897932384626433832795
precision mediump float;

layout(location = 0) in vec3 a_pos;

// per instance attributes:
layout(location = 1) in vec3 spawnPoint;

out vec2 v_texCoords, v_pos;
out vec3 cloudColor;
out float opacity;

uniform mat4 mvp;
uniform vec3 up, right;
uniform float time, cloudOpacity, light;

float random(float x)
{
    return fract(sin(x) * 100.);
}

void main()
{
    float nr = float(gl_InstanceID);

    vec3 pos = vec3(0);
    if (a_pos.x > 0.) pos += right;
    else pos -= right;
    if (a_pos.y > 0.) pos += up;
    else pos -= up;

    float size = (random(nr + 100.) + .5) * 90.;
    pos *= size;
    pos += spawnPoint;

    float cycleTime = 60. * (random(nr) + .5);
    float progress = mod(time + nr * 2., cycleTime) / cycleTime;
    opacity = sin(progress * PI);
    
    v_pos = a_pos.xy;

    vec3 direction = vec3((random(nr + 40.) - .5) * .6, (random(nr + 20.) - .5) * .3, 1);
    // move particle in direction:
    pos += direction * progress * 500.;

    v_texCoords = a_pos.xy * size / 800.;
    // texture scrolling:
    v_texCoords.x += (random(nr + 70.) - .5) * time * .04;
    v_texCoords.y += (random(nr + 30.) - .5) * time * .04;

    pos *= .05; // scale
    gl_Position = (mvp * vec4(pos, 1));
    // fade out when camera comes too close:
    opacity *= min(1., gl_Position.z * .1);
    // fade the entire cloud in and out:
    opacity *= min(1., cloudOpacity * (1. + 8. * random(nr)));

    cloudColor = vec3(1.8 * light, 1.3 * light, light) + vec3(.3 + random(nr + 3.) * .2);
    cloudColor.r = min(1., max(0., cloudColor.r));
    cloudColor.g = min(1., max(0., cloudColor.g));
    cloudColor.b = min(1., max(0., cloudColor.b));
    cloudColor -= min(1., max(0., spawnPoint.z + 100.)) * light * vec3(.2, .12, .1);
}
