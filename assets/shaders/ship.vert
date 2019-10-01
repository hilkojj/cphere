#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 1) in vec3 a_normal;

uniform mat4 mvp;

out vec2 v_uv;
out float ligth;

void main() {
    gl_Position = mvp * vec4(a_pos, 1);
    v_uv = a_uv;
}
