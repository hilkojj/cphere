#version 300 es
layout(location = 0) in vec3 a_pos;

out vec2 v_texCoords;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(a_pos, 1);
    v_texCoords = a_pos.xy * vec2(.5) + vec2(.5);
}