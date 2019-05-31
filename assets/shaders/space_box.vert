#version 330 core
layout(location = 0) in vec3 a_pos;

out vec3 v_texCoords;

uniform mat4 view;

void main()
{
    v_texCoords = a_pos;
    gl_Position = (view * vec4(a_pos, 0)).xyww;
}
