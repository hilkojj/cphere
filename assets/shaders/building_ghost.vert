#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nor;
layout(location = 2) in vec3 a_tan;
layout(location = 3) in vec2 a_uv;

uniform mat4 view;
uniform vec3 sunDir;
uniform float time;

out vec2 v_uv;
out float light, y;

void main() {
    gl_Position = view * vec4(a_pos, 1);
    light = dot((vec4(a_nor, 0)).xyz, sunDir) * .3 + .7;
    v_uv = a_uv;
    y = a_pos.y;
}
