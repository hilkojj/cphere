#version 300 es
layout(location = 0) in vec3 a_pos;

out vec2 v_texCoords;

void main()
{
    gl_Position = vec4(a_pos, 1);
    v_texCoords = a_pos.xy * vec2(.5) + vec2(.5);
}