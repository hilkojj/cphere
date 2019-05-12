#version 430 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_texCoords;

uniform mat4 MVP;
uniform vec3 camPos;
uniform vec3 sunDir;

out vec3 v_normal;
out vec3 v_tangent;
out vec2 v_texCoords;
out vec3 v_camPosTanSpace;
out vec3 v_sunDirTanSpace;
out vec3 v_toCamera;
out mat3 v_fromTanSpace;

void main()
{
    gl_Position = MVP * vec4(a_pos, 1);
    v_normal = a_normal;
    v_tangent = a_tangent;
    v_texCoords = a_texCoords;


    vec3 up = a_normal;
    vec3 tan = a_tangent;
    vec3 bitan = normalize(cross(up, tan));

    mat3 toTanSpace = mat3(
        tan.x, bitan.x, up.x,
        tan.y, bitan.y, up.y,
        tan.z, bitan.z, up.z
    );

    v_camPosTanSpace = toTanSpace * camPos;
    v_sunDirTanSpace = toTanSpace * sunDir;

    v_toCamera = (a_pos - (inverse(MVP) * vec4(0, 0, 0, 1)).xyz);
    v_fromTanSpace = toTanSpace;
}
