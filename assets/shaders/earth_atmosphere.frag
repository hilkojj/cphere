#version 330 core
out vec4 color;

in float v_alpha;

void main()
{
    color = vec4(.5, .65, .8, v_alpha);
}
