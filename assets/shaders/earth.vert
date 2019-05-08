#version 430 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_texCoords;

uniform mat4 MVP;

out vec3 v_normal;
out vec3 v_tangent;
out vec2 v_texCoords;

void main()
{
    gl_Position = MVP * vec4(a_pos, 1);
    v_normal = a_normal;
    v_tangent = a_tangent;
    v_texCoords = a_texCoords;
}
