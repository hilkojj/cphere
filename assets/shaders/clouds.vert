#version 300 es
#define PI 3.1415926535897932384626433832795
precision mediump float;

layout(location = 0) in vec3 a_pos;

// per instance attributes:
layout(location = 1) in vec3 spawnPoint0;
// layout(location = 2) in vec3 spawnPoint1;
// layout(location = 3) in vec3 spawnPoint2;
// layout(location = 4) in vec3 spawnPoint3;

out vec2 v_texCoords, v_pos;
out float progress, opacity;

uniform mat4 mvp;
uniform vec3 up, right;
uniform float time;

float random(float x)
{
    return fract(sin(x)*100.0);
}

void main()
{
    float nr = float(gl_InstanceID);

    vec3 pos = vec3(0);
    if (a_pos.x > 0.) pos += right;
    else pos -= right;
    if (a_pos.y > 0.) pos += up;
    else pos -= up;


    float size = (random(nr + 100.) + .5) * 80.;
    pos *= size;

    pos += spawnPoint0;

    float cycleTime = 60. * (random(nr) + .5);
    progress = mod(time + nr * 2., cycleTime) / cycleTime;
    opacity = sin(progress * PI);
    
    v_pos = a_pos.xy;

    vec3 direction = vec3((random(nr + 40.) - .5) * .3, (random(nr + 20.) - .5) * .3, 1);

    pos += direction * progress * 500.;

    v_texCoords = a_pos.xy * size / 800.;

    // texture scrolling:
    v_texCoords.x += (random(nr + 70.) - .5) * time * .02;
    v_texCoords.y += (random(nr + 30.) - .5) * time * .02;

    pos *= .05;
    gl_Position = (mvp * vec4(pos, 1));
    // fade out when camera comes too close:
    opacity *= min(1., gl_Position.z * .1);
}
