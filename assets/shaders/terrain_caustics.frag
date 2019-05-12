#version 430 core

in vec3 v_normal;
in vec2 v_texCoord;

out vec3 color;

uniform vec3 sunDir;
uniform sampler2D causticsSheet;
uniform sampler2D terrainTexture;
uniform float time;

void main() {

    color = texture2D(terrainTexture, v_texCoord).rgb;

    for (float t = time; t < time + .2; t += .02) {

        float x = floor(mod(t * 12., 6.));
        float y = floor(mod(t * 2., 6.));

        vec2 offset = vec2(x / 6., y / 6.);

        color += texture2D(
            causticsSheet,
            vec2(mod(v_texCoord.x / 1.5 + t * .001, 1./6.), mod(v_texCoord.y / 1.5 + t * .001, 1./6.)) + offset
        ).rgb * .02;
    }

    // light
    float maxDayLight = dot(v_normal * .7 + sunDir * .3, sunDir) * .4 + .6;
    float minDayLight = dot(v_normal, sunDir) * .4 + .6;
    color.r *= maxDayLight;
    color.g *= maxDayLight * .5 + minDayLight * .5;
    color.b *= minDayLight;
}

