#version 330 core
out vec4 color;

in vec3 v_normal;
in vec2 v_texCoords;
in vec3 v_camPosTanSpace;
in vec3 v_sunDirTanSpace;
in vec3 v_toCamera;
in mat3 v_fromTanSpace;
in float v_edge;

uniform vec3 sunDir;
uniform sampler2D seaNormals;
uniform sampler2D seaDUDV;
uniform sampler2D foamTexture;
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

float clamp(float x)
{
    return min(1, max(0, x));
}

void main()
{
    vec2 screenCoords = gl_FragCoord.xy / scrSize;
    float distToSea = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
    float distToSeaBottom = texture2D(underwaterDepthTexture, screenCoords).r;
    distToSeaBottom = 2.0 * near * far / (far + near - (2.0 * distToSeaBottom - 1.0) * (far - near));

    float seaDepth = distToSeaBottom - distToSea;

    if (seaDepth < 0) discard;

    color = vec4(0.12, .13, .27, 1.);

    // create distorted coords:
    float y = v_texCoords.y;
    vec2 distortedTexCoords = sample(seaDUDV, vec2(time * .01, 0), y, 20).rg * .1;
    distortedTexCoords = vec2(distortedTexCoords.x - time * .02, distortedTexCoords.y + sin(time) * .04);
    vec2 totalDistortion = (sample(seaDUDV, distortedTexCoords, y, 20).rg * 2.0 - 1.0) * .02;

    // get normal vector:
    vec3 normal = sample(seaNormals, totalDistortion, y, 30).xyz;
    normal *= 2;
    normal -= 1;
    float normalStrength = max(.1, ((100. - distToSea) / 100.) * .6);
    vec3 strongNormal = normal;
    normal = normal * normalStrength + vec3(0, 0, 1 - normalStrength);
    normal = normalize(normal);

    // waves from heightmap:
    float waveHeight = 1 - texture2D(underwaterTexture, screenCoords).a;

    if (waveHeight > 0)
    {
        const ivec3 off = ivec3(-3, 0, 3);
        const vec2 size = vec2(2.0, 0.0);
        // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
        float s11 = waveHeight;
        float s01 = textureOffset(underwaterTexture, screenCoords, off.xy).a;
        float s21 = textureOffset(underwaterTexture, screenCoords, off.zy).a;
        float s10 = textureOffset(underwaterTexture, screenCoords, off.yx).a;
        float s12 = textureOffset(underwaterTexture, screenCoords, off.yz).a;
        
        vec3 va = normalize(vec3(size.xy, s21 - s01));
        vec3 vb = normalize(vec3(size.yx, s12 - s10));

        vec3 waveNormal = cross(va, vb);
        float waveFactor = clamp(max(0, normal.x + normal.y) * 10 + .5);

        float foam = clamp((waveHeight - .8) / .2) 
                * pow(waveHeight, 3) 
                    * pow(waveHeight, 3) 
                * pow(waveHeight, 3) 
                * sample(foamTexture, vec2(

                    v_texCoords.x * 100, v_texCoords.y * 20

            ) + waveNormal.xy * .1, y, 30).r * waveFactor;

        color.rgb += foam;
        
        waveFactor *= clamp(min(screenCoords.x, 1 - screenCoords.x) / .05);

        normal = normalize(waveNormal * pow(waveHeight, 3) * 20 * clamp((100. - distToSea) / 100.) * waveFactor + normal);
    }

    // fresnel with normal map:
    vec3 viewVector =  normalize(v_camPosTanSpace - normal);
    float fr =  1.0 - dot(normal, viewVector);
    float reduceEdgeFresnel = clamp((distToSea - 90) / 100) * .55;
    color.rgb += clamp(fr * 1.8 - v_edge * reduceEdgeFresnel);

    // diffuse light:
    float lambertTerm = dot(normal, v_sunDirTanSpace);
    color.rgb *= lambertTerm * .3 + .7;

    // specular light 1:
    vec3 reflectDir = reflect(sunDir, strongNormal * v_fromTanSpace);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = clamp(specular);
    float dampedSpec = pow(specular, 100);
    color.rgb += dampedSpec * .1 * vec3(.8, .8, .6);

    // specular light 2:
    reflectDir = reflect(sunDir, normal * v_fromTanSpace);
    specular = dot(reflectDir, normalize(v_toCamera));
    specular = clamp(specular);
    dampedSpec = pow(specular, 600);
    color.rgb += dampedSpec * .4 * vec3(1, .9, .6);

    vec2 dudv = normal.xy * .1;
    vec2 distortedScreenCoords = screenCoords + dudv;
    distortedScreenCoords.x = max(0.01, min(.99, distortedScreenCoords.x));
    distortedScreenCoords.y = max(0.01, min(.99, distortedScreenCoords.y));

    color.a = seaDepth * 4;

    // underwater:
    float distortion = min(1, seaDepth * .5);
    vec3 underWaterColor = texture2D(underwaterTexture, distortedScreenCoords * distortion + screenCoords * (1 - distortion)).rgb;
    if (all(greaterThan(underWaterColor, vec3(.01))))
    {
        float underwaterFactor = max(0, (4 - seaDepth) / 4);

        vec3 blueish = vec3(underWaterColor.r + underWaterColor.g + underWaterColor.b) / 3;
        blueish *= normalize(vec3(.16, .8, 1));

        float blueFactor = clamp(underwaterFactor - .1);
        underWaterColor *= blueFactor;
        underWaterColor += blueish * (1 - blueFactor);


        color.rgb *= 1 - underwaterFactor;
        color.rgb += underwaterFactor * underWaterColor;
    }
}
