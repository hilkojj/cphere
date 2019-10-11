#version 300 es
precision mediump float;
precision highp sampler2DArray;

// in vec3 v_normal, v_tangent;
in vec2 v_texCoord;
in vec4 v_texBlend;

in vec3 v_sunDirTanSpace;

in float v_dayLight;
in vec3 v_toCamera;

out vec4 color;

uniform vec3 sunDir;
uniform float time;
uniform sampler2DArray terrainTextures;
uniform int backgroundTerrainLayer;
uniform vec4 terrainLayers, specularity, textureScale;
uniform ivec4 hasNormal, fadeBlend;

void layer(int i, inout vec3 normal, inout vec4 color, inout float remainingA, inout float specular)
{
    float a = i == -1 ? remainingA : v_texBlend[i];
    if (a == 0.) return;
    float layer = i == -1 ? float(backgroundTerrainLayer) : terrainLayers[i];
    vec2 coord = v_texCoord * vec2(i == -1 ? 1. : textureScale[i]);
    vec4 rgbAndHeight = texture(terrainTextures, vec3(coord, layer));
    vec3 texNormal = (i == -1 || hasNormal[i] == 1) ? texture(terrainTextures, vec3(coord, layer + 1.)).xyz : vec3(.5, .5, 1);

    if (i >= 0)
    {
        if (fadeBlend[i] == 0)
        {
            float l = rgbAndHeight.a * a + .002;
            a = a * a >= l ? 1. : pow(min(1., a + l), 2.);
        }
        a = min(remainingA, a);
    }
    color.rgb += rgbAndHeight.rgb * a;
    normal += texNormal * a;
    remainingA -= a;
    if (i >= 0) specular += a * specularity[i];
}

void main() {
//    discard;
    float remainingA = 1.;
    float specularA = 0.;

    color = vec4(0, 0, 0, 1);
    vec3 normal = vec3(0);

    layer(3, normal, color, remainingA, specularA);
    layer(2, normal, color, remainingA, specularA);
    layer(1, normal, color, remainingA, specularA);
    layer(0, normal, color, remainingA, specularA);
    layer(-1, normal, color, remainingA, specularA);
        
    normal *= 2.;
    normal -= 1.;

    // light
    float diffuseLight = dot(normal, v_sunDirTanSpace) * .4 + .6;
//    float normalMapEffect = clamp(v_dayLight - .3, 0., 1.);
    color.rgb *= diffuseLight;

    vec3 reflectDir = reflect(v_sunDirTanSpace, normal);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = min(1., max(0., specular));
    float dampedSpec = pow(specular, 10.);
    color.rgb += dampedSpec * specularA;

    // color.rgb = vec3(v_dayLight);
}

