#version 330 core
out vec4 color;

in vec3 v_color;
in float v_alpha;

void main()
{
    color = vec4(v_color, v_alpha);
}
