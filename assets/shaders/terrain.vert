#version 300 es
precision mediump float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoords;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec4 a_texBlend;

uniform mat4 viewTrans;
uniform vec3 sunDir;

out vec2 v_texCoord;
out vec4 v_texBlend;

out vec3 v_sunDirTanSpace;

void main() {
    gl_Position = viewTrans * vec4(a_pos, 1);
    v_texCoord = a_texCoords * 4.;
    v_texBlend = a_texBlend;

    vec3 up = a_normal;
    vec3 tan = a_tangent;
    vec3 bitan = normalize(cross(up, tan));

    mat3 toTanSpace = mat3(
        tan.x, bitan.x, up.x,
        tan.y, bitan.y, up.y,
        tan.z, bitan.z, up.z
    );

    v_sunDirTanSpace = toTanSpace * sunDir;

}
