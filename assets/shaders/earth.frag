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
in float v_shadowOpacity;

in vec4 shadowMapCoords;

uniform vec3 sunDir;
uniform sampler2D foamTexture;
uniform sampler2D seaWaves;
uniform sampler2D underwaterTexture;
uniform sampler2D underwaterDepthTexture;
uniform sampler2D reflectionTexture;

uniform lowp sampler2DShadow shadowBuffer;

uniform float time;
uniform vec2 scrSize;

const float pole = .2, poleMargin = .1, near = .1, far = 1000.;

float clamp1(float x)
{
    return max(min(1., x), 0.);
}

/**
 * A function that samples a texture in a weird way.
 * The poles of the earth use different texture coordinates.
 */
vec4 sampleTex(sampler2D tex, vec2 offset, float y, float scale)
{
    y = abs(y * 2. - 1.);
    float weirdness = 0.;
    vec4 c = vec4(0);
    bool inPole = y > 1. - pole;
    if (inPole || y > 1. - pole - poleMargin)
    {
        c = texture(tex, (v_normal.xz * scale * .3) + offset);
        if (inPole) return c;

        weirdness = y - (1. - pole - poleMargin);
        weirdness /= pole - poleMargin;
        c *= weirdness;
    }
    return c + texture(tex, (v_texCoords * scale) + offset) * (1. - weirdness);
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

// similar to sampleTex() but for seaHeight
float seaHeightSphere(vec2 off)
{
    float y = abs(v_texCoords.y * 2. - 1.);
    float weirdness = 0.;
    float height = 0.;
    bool inPole = y > 1. - pole;
    if (inPole || y > 1. - pole - poleMargin)
    {
        height = seaHeight(v_normal.xz * 30. + off);
        if (inPole) return height;

        weirdness = y - (1. - pole - poleMargin);
        weirdness /= pole - poleMargin;
        height *= weirdness;
    }

    float height0 = 0., weirdness0 = 0.;

    if (v_texCoords.x > .98)
    {
        float x = v_texCoords.x - 1.;

        weirdness0 = (v_texCoords.x - .98) * 50.;

        height0 = (seaHeight(vec2(x, v_texCoords.y) * 100. + off) * weirdness0);
    }

    height0 += seaHeight(v_texCoords * 100. + off) * (1. - weirdness0);

    return height + height0 * (1. - weirdness);
}

void normalAndHeight(out vec3 normalDetailed, out vec3 normal, out float height, float detail)
{
    const vec3 off = vec3(-.03, 0, .03);
    // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
    float s11 = height = seaHeightSphere(vec2(0));
    float s01 = seaHeightSphere(off.xy);
    float s21 = seaHeightSphere(off.zy);
    float s10 = seaHeightSphere(off.yx);
    float s12 = seaHeightSphere(off.yz);
    
    vec2 sizeDetailed = vec2(.15 - .07 * detail, 0.); // .06
    vec3 va = normalize(vec3(sizeDetailed.xy, s21 - s01));
    vec3 vb = normalize(vec3(sizeDetailed.yx, s12 - s10));

    normalDetailed = cross(va, vb);

    vec2 size = vec2(1., 0.); // .13
    va = normalize(vec3(size.xy, s21 - s01));
    vb = normalize(vec3(size.yx, s12 - s10));
    normal = cross(va, vb);
}

vec3 specular(vec3 normal, float seaHeight, float detail)
{
    vec3 normallll = normal * .8 + vec3(0, 0, .2);

    vec3 reflectDir = reflect(sunDir, normallll * v_fromTanSpace);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = clamp1(specular);
    float dampedSpec = pow(specular, 200.);
    return dampedSpec * vec3(1., .95, .9);
}

float foamAndWaves(inout vec3 normal, inout vec3 normalDetailed, vec2 screenCoords, float detail, float seaHeight, float seaDepth)
{
    // waves from heightmap:
    float waveHeight = 1. - texture(underwaterTexture, screenCoords).a;

    float foam = clamp1(pow(clamp1((seaHeight - 1.23) * 8.), 3.));
    vec3 waveNormal = vec3(0, 0, 1);

    if (waveHeight > 0.)
    {
        const ivec3 off = ivec3(-7, 0, 7); // 7 is max on most devices
        vec2 size = vec2(4., 0.0);
        // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
        float s11 = waveHeight;
        float s01 = 1. - textureOffset(underwaterTexture, screenCoords, off.xy).a;
        float s21 = 1. - textureOffset(underwaterTexture, screenCoords, off.zy).a;
        float s10 = 1. - textureOffset(underwaterTexture, screenCoords, off.yx).a;
        float s12 = 1. - textureOffset(underwaterTexture, screenCoords, off.yz).a;
        
        vec3 va = normalize(vec3(size.xy, s21 - s01));
        vec3 vb = normalize(vec3(size.yx, s12 - s10));

        waveNormal = cross(va, vb);

        foam += pow(waveHeight, 2.);
        
        normal = waveNormal * waveHeight + normal * (1. - waveHeight);//normalize(normal);

        normalDetailed = waveNormal * waveHeight + normalDetailed * (1. - waveHeight);//normalize(normal);
    }
    // foam = clamp1(foam + clamp1(1. - seaDepth * 4. + normalDetailed.x - seaHeight * seaHeight * .2));
    return foam;
}

void applyFoam(float foam, vec3 normal)
{
    if (foam > .0)
    {
        float y = v_texCoords.y;
        float originalFoam = foam;

        foam *= sampleTex(foamTexture, normal.xy * .05 + time * .01, y, 160.).r;
        foam += sampleTex(foamTexture, normal.xy * .05 + time * -.2, y, 300.).r * (1. - foam) * originalFoam * originalFoam;
        foam = clamp1(foam);

        float blub = pow(foam, 1.5);
        vec3 foamColor = vec3(.35, .45, .42) * (1. - blub) + blub;
        float lambertTerm = clamp1(dot(normal, v_sunDirTanSpace)) * .6 + .4;

        foamColor *= lambertTerm;
        color.rgb *= 1. - foam;
        color.rgb += foamColor * foam;
    }
}

void underwater(float seaDepth, vec3 normal, vec2 screenCoords, float visibility)
{
    seaDepth += .1;
    vec2 dudv = normal.xy * .15;
    vec2 distortedScreenCoords = screenCoords + dudv;
    distortedScreenCoords.x = max(0.01, min(.99, distortedScreenCoords.x));
    distortedScreenCoords.y = max(0.01, min(.99, distortedScreenCoords.y));

    float distortion = min(1., seaDepth + .3);
    vec3 underWaterColor = texture(underwaterTexture, distortedScreenCoords * distortion + screenCoords * (1. - distortion)).rgb;
    if (all(greaterThan(underWaterColor, vec3(.01))))
    {
        float underwaterFactor = max(0., (4.3 - seaDepth) / 4.3);

        vec3 blueish = vec3(underWaterColor.r + underWaterColor.g + underWaterColor.b) / 3.;
        blueish *= normalize(vec3(.1, 1., 1.));

        float blueFactor = clamp1(underwaterFactor - .1);
        underWaterColor *= blueFactor;
        underWaterColor += blueish * (1. - blueFactor);
        underwaterFactor *= visibility;


        color.rgb *= 1. - underwaterFactor;
        color.rgb += underwaterFactor * underWaterColor;
    }
}

float fresnelReflection(vec3 normal, float detail, float daylight, vec2 screenCoords)
{
    float dayLightEffect = (1. - detail) * .7 + .3;
    vec3 reflectionColor = vec3(.45, .55, .9);

    reflectionColor *= daylight * dayLightEffect + (1. - dayLightEffect);

    reflectionColor *= detail * .2 + .8;

    vec3 viewVector =  normalize(v_camPosTanSpace - normal);
    float fr = 1.0 - dot(normal, viewVector);
    fr *= 4. * detail + 1.;
    fr = clamp1(fr);
    color.rgb += reflectionColor * fr;

    vec3 refTexCol = texture(reflectionTexture, screenCoords + normal.xy * .2).rgb;
    float reflectionFactor = clamp1(clamp1(pow(fr * 20. + .4, 1.)) - .6);

    reflectionFactor *= clamp1((detail - .25) * 5. + daylight * 2.);

    color.rgb *= 1. - reflectionFactor;
    color.rgb += reflectionFactor * refTexCol;
    return fr;
}

void main()
{
//     discard;
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

    // diffuse light:
    float lambertTerm = clamp1(dot(normal, v_sunDirTanSpace));
    color.rgb *= lambertTerm * .4 + .6;

    // shore (and boat-wake?) waves+foam:
    float foam = foamAndWaves(normal, normalDetailed, screenCoords, detail, seaHeight, seaDepth);
    // foam = 1.;

    // reflection:
    float fresnel = fresnelReflection(normal, detail, daylight, screenCoords);

    // underwater:
    underwater(seaDepth, normal, screenCoords, clamp1(1. - fresnel - clamp1(.1 - detail)));

    applyFoam(foam, normal);


    float shadow = 0.;
    if (v_shadowOpacity > 0. && shadowMapCoords.x >= 0. && shadowMapCoords.x <= 1. && shadowMapCoords.y >= 0. && shadowMapCoords.y <= 1.)
    {
        shadow = texture(shadowBuffer, shadowMapCoords.xyz);

        float shadowOpacity = v_shadowOpacity * .3;

        color.rgb *= shadow * shadowOpacity + (1. - shadowOpacity);
    }

    // specular:
    color.rgb += specular(normalDetailed, seaHeight, detail) * (shadow);

    // fade edges:
    color.a = seaDepth * 3. > normalDetailed.x ? 1. : .4;
    color.a *= seaDepth * 10.;

//    color.rgb = vec3(v_shadowOpacity);
}
