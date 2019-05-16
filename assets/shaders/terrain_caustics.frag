#version 430 core

in vec3 v_normal;
in vec2 v_texCoord;

out vec4 color;

uniform vec3 sunDir;
uniform sampler2D causticsSheet;
uniform sampler2D terrainTexture;
uniform float time;

void main() {

    color = texture2D(terrainTexture, v_texCoord);

    // light
    float dayLight = dot(v_normal, sunDir) * .3 + .7;

    for (float t = time; t < time + .2; t += .02) {

        float x = floor(mod(t * 12., 6.));
        float y = floor(mod(t * 2., 6.));

        vec2 offset = vec2(x / 6., y / 6.);

        color.rgb += texture2D(
            causticsSheet,
            vec2(mod(v_texCoord.x + t * .001, 1./6.), mod(v_texCoord.y + t * .001, 1./6.)) + offset
        ).rgb * .017 * dayLight;
    }
    
    color.rgb *= dayLight;
}

