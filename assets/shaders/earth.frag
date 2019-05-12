#version 330 core
out vec4 color;

in vec3 v_normal;
in vec3 v_tangent;
in vec2 v_texCoords;
in vec3 v_camPosTanSpace;
in vec3 v_sunDirTanSpace;
in vec3 v_toCamera;
in mat3 v_fromTanSpace;

uniform vec3 sunDir;
uniform sampler2D seaNormals;
uniform sampler2D seaDUDV;
uniform float time;

const float pole = .2, poleMargin = .1;

/**
 * A function that samples a texture in a weird way.
 * The poles of the earth use different texture coordinates.
 */
vec4 sample(sampler2D tex, vec2 coords, float y, float scale)
{
    y = abs(y * 2 - 1);
    float weirdness = 0;
    vec4 c = vec4(0);
    bool inPole = y > 1 - pole;
    if (inPole || y > 1 - pole - poleMargin)
    {
        c = texture(tex, (v_normal.xz * scale * .2) + coords);
        if (inPole) return c;

        weirdness = y - (1 - pole - poleMargin);
        weirdness /= pole - poleMargin;
        c *= weirdness;
    }
    return c + texture(tex, (v_texCoords * scale) + coords) * (1 - weirdness);
}

void main()
{
    vec4 outColor = vec4(0.1, .23, .44, 1.);

    float y = v_texCoords.y;
    vec2 distortedTexCoords = sample(seaDUDV, vec2(time * .01, 0), y, 20).rg * .1;
    distortedTexCoords = vec2(distortedTexCoords.x - time * .02, distortedTexCoords.y + sin(time) * .01);
    vec2 totalDistortion = (sample(seaDUDV, distortedTexCoords, y, 15).rg * 2.0 - 1.0) * .02;

    vec3 normal = sample(seaNormals, totalDistortion, y, 15).xyz;
    normal *= 2;
    normal -= 1;
    normal = normal * .3 + vec3(0, 0, 1) * .7;
    normal = normalize(normal);

    // fresnel with normal map
    vec3 viewVector =  normalize(v_camPosTanSpace - normal);
    float fr =  1.0 - dot(normal, viewVector);
    outColor += fr * 10;//max(0., fr - v_edge * .3);

    float lambertTerm = dot(normal, v_sunDirTanSpace);
    outColor.rgb *= lambertTerm * .4 + .6;

    vec3 reflectDir = reflect(sunDir, normal * v_fromTanSpace);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = max(0, specular);
    float dampedSpec = pow(specular, 300);

    color = outColor + dampedSpec;
}
