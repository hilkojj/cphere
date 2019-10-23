#version 300 es
precision mediump float;

in vec3 v_normal;
in vec2 v_texCoord;
in float v_y;

out vec4 color;

uniform vec3 sunDir;
uniform sampler2D causticsSheet;
uniform sampler2D terrainTexture;
uniform float time;

const float near = .1, far = 1000.;

void main() {

    if (v_y > 4.) discard;

    color = texture(terrainTexture, v_texCoord);

    // light
    float dayLight = dot(v_normal, sunDir) * .33 + .67;
    float dist = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));

    for (float t = time; t < time + .2; t += .02) {

        float x = floor(mod(t * 12., 6.));
        float y = floor(mod(t * 2., 6.));

        vec2 offset = vec2(x / 6., y / 6.);

        color.rgb += texture(
            causticsSheet,
            vec2(mod(v_texCoord.x * .16 + t * .001, 1./6.), mod(v_texCoord.y * .16 + t * .001, 1./6.)) + offset
        ).rgb * .027 * max(.6, min(1., 1. - (dist - 20.) / 90.)) * dayLight;
    }
    
    color.rgb *= dayLight * .9;
}

