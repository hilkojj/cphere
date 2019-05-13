#version 330 core
out vec4 color;

in vec3 v_normal;
in vec3 v_tangent;
in vec2 v_texCoords;
in vec3 v_camPosTanSpace;
in vec3 v_sunDirTanSpace;
in vec3 v_toCamera;
in mat3 v_fromTanSpace;
in float v_edge;

uniform vec3 sunDir;
uniform sampler2D seaNormals;
uniform sampler2D seaDUDV;
uniform sampler2D underwaterTexture;
uniform sampler2D underwaterDepthTexture;
uniform float time;
uniform vec2 scrSize;

const float pole = .2, poleMargin = .1, near = .1, far = 1000;

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
    vec2 screenCoords = gl_FragCoord.xy / scrSize;
    float distToSea = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
    float distToSeaBottom = texture2D(underwaterDepthTexture, screenCoords).r;
    distToSeaBottom = 2.0 * near * far / (far + near - (2.0 * distToSeaBottom - 1.0) * (far - near));

    float seaDepth = distToSeaBottom - distToSea;

    if (seaDepth < 0) discard;

    color = vec4(0.06, .16, .3, 1.);

    // create distorted coords:
    float y = v_texCoords.y;
    vec2 distortedTexCoords = sample(seaDUDV, vec2(time * .01, 0), y, 60).rg * .1;
    distortedTexCoords = vec2(distortedTexCoords.x - time * .02, distortedTexCoords.y + sin(time) * .04);
    vec2 totalDistortion = (sample(seaDUDV, distortedTexCoords, y, 60).rg * 2.0 - 1.0) * .02;

    // get normal vector:
    vec3 normal = sample(seaNormals, totalDistortion, y, 20).xyz;
    normal *= 2;
    normal -= 1;
    float normalStrength = max(.2, ((100. - distToSea) / 100.) * .5);
    normal = normal * normalStrength + vec3(0, 0, 1 - normalStrength);
    normal = normalize(normal);

    // fresnel with normal map:
    vec3 viewVector =  normalize(v_camPosTanSpace - normal);
    float fr =  1.0 - dot(normal, viewVector);
    color.rgb += max(0, fr * 2 - v_edge * .7);

    // diffuse light:
    float lambertTerm = dot(normal, v_sunDirTanSpace);
    color.rgb *= lambertTerm * .3 + .7;

    // specular light:
    vec3 reflectDir = reflect(sunDir, normal * v_fromTanSpace);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = max(0, specular);
    float dampedSpec = pow(specular, 200);
    color.rgb += dampedSpec * dampedSpec * vec3(1, .9, .6);

    vec2 dudv = normal.xy * .1;
    vec2 distortedScreenCoords = screenCoords + dudv;
    distortedScreenCoords.x = max(0.01, min(.99, distortedScreenCoords.x));
    distortedScreenCoords.y = max(0.01, min(.99, distortedScreenCoords.y));

    color.a = seaDepth * 2;

    // underwater:
    float distortion = min(1, seaDepth * .5);
    vec3 underWaterColor = texture2D(underwaterTexture, distortedScreenCoords * distortion + screenCoords * (1 - distortion)).rgb;
    if (all(greaterThan(underWaterColor, vec3(.01))))
    {
        float underwaterFactor = max(0, (2 - seaDepth) / 2);

        vec3 blueish = vec3(underWaterColor.r + underWaterColor.g + underWaterColor.b) / 3;
        blueish *= normalize(vec3(.16, .8, 1));

        underWaterColor *= underwaterFactor;
        underWaterColor += blueish * (1 - underwaterFactor);

        color.rgb *= 1 - underwaterFactor;
        color.rgb += underwaterFactor * underWaterColor;
    }
}
