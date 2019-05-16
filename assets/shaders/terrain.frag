#version 430 core

// in vec3 v_normal, v_tangent;
in vec2 v_texCoord;
in vec4 v_texBlend;

in vec3 v_sunDirTanSpace;

out vec3 color;

uniform vec3 sunDir;
uniform float time;
uniform sampler2DArray terrainTextures;
uniform int backgroundTerrainLayer;
uniform vec4 terrainLayers, hasNormal;

void main() {
    float remainingA = 1;

    color = vec3(0);
    vec3 normal = vec3(0);

    for (int i = 3; i >= -1 && remainingA > 0; i--)
    {
        float a = i == -1 ? remainingA : v_texBlend[i];
        float layer = i == -1 ? backgroundTerrainLayer : terrainLayers[i];

        vec4 rgbAndHeight = texture(terrainTextures, vec3(v_texCoord, layer));
        vec3 texNormal = (i == -1 || hasNormal[i] > .5) ? texture(terrainTextures, vec3(v_texCoord, layer + 1)).xyz : vec3(.5, .5, 1);

        if (i >= 0)
        {
            float a0 = (rgbAndHeight.a * 2. - 1.) * (1. - a);
            if (a < .1) a0 *= a * 10.;
            a += a0;
            if (a < 0.) a = 0.;

            a = min(remainingA, a);
        }
        color += rgbAndHeight.rgb * a;
        normal += texNormal * a;
        remainingA -= a;
    }
    normal *= 2;
    normal -= 1;

    // light
    float dayLight = dot(normal, v_sunDirTanSpace) * .3 + .7;
    color.rgb *= dayLight;
}

