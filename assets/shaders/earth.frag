#version 300 es
precision highp float;

out vec4 color;

in vec3 v_pos;
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

uniform sampler2D seaWaves;

uniform float time;
uniform vec2 scrSize;

float wavePattern(vec2 uv, float detail)
{
    return pow(texture(seaWaves, uv * .05).r, detail);
}

float seaHeight(vec2 uv)
{
    float scale = .24, strength = .5, detail = 3., height = 0.;

    for (int i = 0; i < 5; i++)
    {
    	float p = wavePattern((uv + time * .2) * scale, detail) + wavePattern((uv - time * .3) * scale, detail);
        
        height += p * strength;
        
        const mat2 magic = mat2(1.7, 1.2, -1.2, 1.4);
    	uv *= magic;
        
        scale *= 1.9;
        strength *= 0.32;
        detail = mix(detail, 1., .2);
    }
    return height;
}

void normalAndHeight(out vec3 normalDetailed, out vec3 normal)
{
    const vec3 off = vec3(-.01, 0, .01);
    vec2 uv = v_texCoords * 200.;
    // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
    float s11 = seaHeight(uv);
    float s01 = seaHeight(uv + off.xy);
    float s21 = seaHeight(uv + off.zy);
    float s10 = seaHeight(uv + off.yx);
    float s12 = seaHeight(uv + off.yz);
    
    const vec2 sizeDetailed = vec2(.07, 0.);
    vec3 va = normalize(vec3(sizeDetailed.xy, s21 - s01));
    vec3 vb = normalize(vec3(sizeDetailed.yx, s12 - s10));

    normalDetailed = cross(va, vb);

    const vec2 size = vec2(.13, 0.);
    va = normalize(vec3(size.xy, s21 - s01));
    vb = normalize(vec3(size.yx, s12 - s10));
    normal = cross(va, vb);
}

void main()
{
    vec3 normal, normalDetailed;
    normalAndHeight(normalDetailed, normal);

    color = vec4(0.12, .13, .27, 1.);

    float lambertTerm = dot(normal, v_sunDirTanSpace);
    color.rgb *= lambertTerm * .3 + .7;

    // fresnel:
    vec3 viewVector =  normalize(v_camPosTanSpace - normal);
    float fr =  1.0 - dot(normal, viewVector);
    color.rgb += clamp(fr * .85, 0, 1);

    // specular:
    vec3 reflectDir = reflect(sunDir, normalDetailed * v_fromTanSpace);
    float specular = dot(reflectDir, normalize(v_toCamera));
    specular = clamp(specular, 0, 1);
    float dampedSpec = pow(specular, 200.);
    color.rgb += dampedSpec * .5 * vec3(1., .9, .7);

}
