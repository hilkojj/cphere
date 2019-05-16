#version 330 core
out vec4 color;

in vec3 v_normal;
in vec2 v_texCoords;
in vec3 v_camPosTanSpace;
in vec3 v_sunDirTanSpace;
in vec3 v_toCamera;
in mat3 v_fromTanSpace;
in float v_edge;

void main()
{
    color = vec4(0, 1, 0, 1 - v_edge);
}
