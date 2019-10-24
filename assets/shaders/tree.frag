#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;
in float light, y;
flat in float v_random;
flat in int instanceId;

uniform sampler2D buildingTexture;

uniform int justPlacedId;
uniform vec4 timeSincePlacing;



void main()
{
    color = texture(buildingTexture, v_uv);

    if (color.a <= .7) discard;
//    color.a *= 3.;


    if (instanceId >= justPlacedId)
    {
        // placing animation:

        float animTime = clamp(timeSincePlacing[instanceId - justPlacedId] - .1, 0., .5) * 2.;

        if (color.g > color.r) {
            if (y < (1. - animTime) * 10.)
                discard;
        }
    }

    color.rgb *= light;

    if (color.g < color.r) return; // only change color of leaves

    float r = v_random * v_random * v_random;

    vec3 c = v_random > .5 ? vec3(.8, 1.5, 1.4) : vec3(15., 1., 1.4);

    color.rgb *= c * r + vec3(1. - r);

    r *= .5;
    color.rgb = (vec3(color.r + color.g + color.b) / 3.) * r + color.rgb * (1. - r);
}
