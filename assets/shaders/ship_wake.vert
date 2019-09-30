#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

uniform mat4 viewTrans;

out vec2 v_uv;

void main() {
    gl_Position = viewTrans * vec4(a_pos, 1);
    v_uv = a_uv;
}
