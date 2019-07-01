#version 300 es
precision mediump float;
precision highp sampler2DArray;

// in vec3 v_normal, v_tangent;
in vec2 v_texCoord;
in vec4 v_texBlend;

in vec3 v_sunDirTanSpace;

out vec4 color;

uniform vec3 sunDir;
uniform float time;
uniform sampler2DArray terrainTextures;
uniform int backgroundTerrainLayer;
uniform vec4 terrainLayers, hasNormal;

void layer(int i, inout vec3 normal, inout vec4 color, inout float remainingA)
{
    float a = i == -1 ? remainingA : v_texBlend[i];
    if (a == 0.) return;
    float layer = i == -1 ? float(backgroundTerrainLayer) : terrainLayers[i];
    vec4 rgbAndHeight = texture(terrainTextures, vec3(v_texCoord, layer));
    vec3 texNormal = (i == -1 || hasNormal[i] > .5) ? texture(terrainTextures, vec3(v_texCoord, layer + 1.)).xyz : vec3(.5, .5, 1);

    if (i >= 0)
    {
        float a0 = rgbAndHeight.a * (1. - a);
        if (a < .1) a0 *= a * 5.;
        a += a0;
        if (a < 0.) a = 0.;

        a = min(remainingA, a);
    }
    color.rgb += rgbAndHeight.rgb * a;
    normal += texNormal * a;
    remainingA -= a;
}

void main() {
    float remainingA = 1.;

    color = vec4(0, 0, 0, 1);
    vec3 normal = vec3(0);

    layer(3, normal, color, remainingA);
    layer(2, normal, color, remainingA);
    layer(1, normal, color, remainingA);
    layer(0, normal, color, remainingA);
    layer(-1, normal, color, remainingA);
        
    normal *= 2.;
    normal -= 1.;

    // light
    float dayLight = dot(normal, v_sunDirTanSpace) * .3 + .7;
    color.rgb *= dayLight;
}

