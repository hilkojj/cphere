#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;

uniform mat4 mvp;
uniform vec3 sunDir;
uniform bool reflection;

out vec2 v_uv;
out float light;

void main() {
    vec3 pos = a_pos;
    if (reflection) pos.y *= -1.;
    gl_Position = mvp * vec4(pos, 1);
    light = dot(a_normal, sunDir) * .3 + .7;

    v_uv = a_uv;
}
