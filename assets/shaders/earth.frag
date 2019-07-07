#version 300 es
precision highp float;

out vec4 color;

// in vec3 v_pos;
in vec3 v_normal;
in vec2 v_texCoords;
in vec3 v_camPosTanSpace;
in vec3 v_sunDirTanSpace;
in vec3 v_toCamera;
in mat3 v_fromTanSpace;
in float v_edge;

uniform vec3 sunDir;
// uniform sampler2D seaNormals;
// uniform sampler2D seaDUDV;
uniform sampler2D foamTexture;
uniform sampler2D underwaterTexture;
uniform sampler2D underwaterDepthTexture;
uniform sampler2D seaWaves;

uniform float time;
uniform vec2 scrSize;

const float pole = .2, poleMargin = .1, near = .1, far = 1000.;

float clamp1(float x)
{
    return clamp(x, 0, 1);
}

/**
 * A function that samples a texture in a weird way.
 * The poles of the earth use different texture coordinates.
 */
vec4 sampleTex(sampler2D tex, vec2 coords, float y, float scale)
{
    y = abs(y * 2. - 1.);
    float weirdness = 0.;
    vec4 c = vec4(0);
    bool inPole = y > 1. - pole;
    if (inPole || y > 1. - pole - poleMargin)
    {
        c = texture(tex, (v_normal.xz * scale * .2) + coords);
        if (inPole) return c;

        weirdness = y - (1. - pole - poleMargin);
        weirdness /= pole - poleMargin;
        c *= weirdness;
    }
    return c + texture(tex, (v_texCoords * scale) + coords) * (1. - weirdness);
}

float wavePattern(vec2 uv, float detail)
{
    return pow(texture(seaWaves, uv * .05).r, detail);
}

float seaHeight(vec2 uv)
{
    float scale = .24, strength = .5, detail = 3., height = 0.;

    for (int i = 0; i < 5; i++)
    {
    	float p = wavePattern((uv + time * .2) * scale, detail) + wavePattern((uv - time * .1) * scale, detail);
        
        height += p * strength;
        
        const mat2 magic = mat2(1.7, 1.2, -1.2, 1.4);
    	uv *= magic;
        
        scale *= 1.9;
        strength *= 0.32;
        detail = mix(detail, 1., .2);
    }
    return height;
}

void normalAndHeight(out vec3 normalDetailed, out vec3 normal, out float height, float detail)
{
    const vec3 off = vec3(-.02, 0, .02);
    vec2 uv = v_texCoords * 100.;
    // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
    float s11 = height = seaHeight(uv);
    float s01 = seaHeight(uv + off.xy);
    float s21 = seaHeight(uv + off.zy);
    float s10 = seaHeight(uv + off.yx);
    float s12 = seaHeight(uv + off.yz);
    
    vec2 sizeDetailed = vec2(.15 - .09 * detail, 0.); // .06
    vec3 va = normalize(vec3(sizeDetailed.xy, s21 - s01));
    vec3 vb = normalize(vec3(sizeDetailed.yx, s12 - s10));

    normalDetailed = cross(va, vb);

    vec2 size = vec2(1., 0.); // .13
    va = normalize(vec3(size.xy, s21 - s01));
    vb = normalize(vec3(size.yx, s12 - s10));
    normal = cross(va, vb);
}

vec3 specular(vec3 normal, float seaHeight)
{
    vec3 normallll = normal * .8 + vec3(0, 0, .2);
    vec3 reflectDir = reflect(sunDir, normallll * v_fromTanSpace);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = clamp(specular, 0, 1);
    float dampedSpec = pow(specular, 200.);
    return dampedSpec * vec3(1., .95, .9);
}

void foamAndWaves(inout vec3 normal, inout vec3 normalDetailed, vec2 screenCoords, float detail, float seaHeight, float seaDepth)
{
    float y = v_texCoords.y;

    // waves from heightmap:
    float waveHeight = 1. - texture(underwaterTexture, screenCoords).a;

    float foam = clamp1(pow(clamp1((seaHeight - 1.23) * 6.), 5.));
    vec3 waveNormal = vec3(0, 0, 1);

    if (waveHeight > 0.)
    {
        const ivec3 off = ivec3(-10, 0, 10);
        vec2 size = vec2(1.2, 0.0);
        // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
        float s11 = waveHeight;
        float s01 = 1. - textureOffset(underwaterTexture, screenCoords, off.xy).a;
        float s21 = 1. - textureOffset(underwaterTexture, screenCoords, off.zy).a;
        float s10 = 1. - textureOffset(underwaterTexture, screenCoords, off.yx).a;
        float s12 = 1. - textureOffset(underwaterTexture, screenCoords, off.yz).a;
        
        vec3 va = normalize(vec3(size.xy, s21 - s01));
        vec3 vb = normalize(vec3(size.yx, s12 - s10));

        waveNormal = cross(va, vb);
        float waveFactor = clamp1(max(0., normal.x + normal.y) * 10. + .5);

        foam += waveHeight;
        
        waveFactor *= clamp1(min(screenCoords.x, 1. - screenCoords.x) / .05);

        normal.xy += waveNormal.xy * 1.2 * pow(waveHeight, 1.1) * detail;
        normal = normalize(normal);

        normalDetailed.xy += waveNormal.xy * 2. * waveHeight * detail;
        normalDetailed = normalize(normalDetailed);
    }

    if (foam > .0)
    {
        foam *= seaHeight;

        foam *= sampleTex(foamTexture, vec2(

                    v_texCoords.x * 100., v_texCoords.y * 20.

                ) + waveNormal.xy * .02 + normal.xy * .03, y, 60.).r;
        color.rgb += foam;
    }
    // color.rgb = waveNormal;
}

void underwater(float seaDepth, vec3 normal, vec2 screenCoords, float visibility)
{
    vec2 dudv = normal.xy * .06;
    vec2 distortedScreenCoords = screenCoords + dudv;
    distortedScreenCoords.x = max(0.01, min(.99, distortedScreenCoords.x));
    distortedScreenCoords.y = max(0.01, min(.99, distortedScreenCoords.y));

    float distortion = min(1., seaDepth);
    vec3 underWaterColor = texture(underwaterTexture, distortedScreenCoords * distortion + screenCoords * (1. - distortion)).rgb;
    if (all(greaterThan(underWaterColor, vec3(.01))))
    {
        float underwaterFactor = max(0., (4. - seaDepth) / 4.);

        vec3 blueish = vec3(underWaterColor.r + underWaterColor.g + underWaterColor.b) / 3.;
        blueish *= normalize(vec3(.16, .8, 1));

        float blueFactor = clamp(underwaterFactor - .1, 0, 1);
        underWaterColor *= blueFactor;
        underWaterColor += blueish * (1. - blueFactor);
        underwaterFactor *= visibility;


        color.rgb *= 1. - underwaterFactor;
        color.rgb += underwaterFactor * underWaterColor;
    }
}

float fresnelReflection(vec3 normal, float detail, float daylight)
{
    float dayLightEffect = (1. - detail) * .7 + .3;
    vec3 reflectionColor = vec3(.5, .7, .9);

    reflectionColor *= daylight * dayLightEffect + (1. - dayLightEffect);

    reflectionColor *= detail * .2 + .8;

    reflectionColor *= (1. - v_edge * pow(1. - detail, 5.)) * .7 + .3;

    vec3 viewVector =  normalize(v_camPosTanSpace - normal);
    float fr =  1.0 - dot(normal, viewVector);
    fr *= 2. * detail + 1.5;
    fr = clamp1(fr);
    color.rgb += reflectionColor * fr;
    return fr;
}

void main()
{
    vec2 screenCoords = gl_FragCoord.xy / scrSize;

    float distToSea = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
    float distToSeaBottom = texture(underwaterDepthTexture, screenCoords).r;
    distToSeaBottom = 2.0 * near * far / (far + near - (2.0 * distToSeaBottom - 1.0) * (far - near));

    float seaDepth = distToSeaBottom - distToSea;

    if (seaDepth < 0.) discard; // don't render sea that is under land

    float detail = 1. - clamp1((distToSea - 20.) / 150.);

    // normal and height of sea:
    vec3 normal, normalDetailed;
    float seaHeight;
    normalAndHeight(normalDetailed, normal, seaHeight, detail);

    // sea color:
    color = vec4(0.12, .15, .29, 1.) * seaHeight;
    color += vec4(0.15, .14, .3, 1.) * (1. - seaHeight);

    // daylight:
    float daylight = clamp1(dot(vec3(0, 0, 1), v_sunDirTanSpace) + .4);

    // shore (and boat-wake?) waves+foam:
    foamAndWaves(normal, normalDetailed, screenCoords, detail, seaHeight, seaDepth);

    // diffuse light:
    float lambertTerm = clamp1(dot(normal, v_sunDirTanSpace));
    color.rgb *= lambertTerm * .4 + .6;

    // reflection:
    float fresnel = fresnelReflection(normal, detail, daylight);

    // underwater:
    underwater(seaDepth, normal, screenCoords, clamp1(1. - fresnel - clamp1(.1 - detail)));

    // specular:
    color.rgb += specular(normalDetailed, seaHeight);

    // fade edges:
    color.a = seaDepth * 2.;

    // color.rgb = vec3(seaHeight);
}
