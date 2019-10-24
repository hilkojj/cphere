#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;
in float light, v_random, y;
flat in int instanceId;

uniform sampler2D buildingTexture;

uniform int justPlacedId;
uniform vec4 timeSincePlacing;

void main()
{
    color = texture(buildingTexture, v_uv);
    if (color.a <= .7) discard;
    color.a *= 3.;

//    if (instanceId >= justPlacedId)
//    {
//        // placing animation:
//        float animTime = clamp(timeSincePlacing[instanceId - justPlacedId], 0., .5) * 2.;
//        if (10. - y > animTime * 10.)
//            discard;
//    }

    color.rgb *= light;
}
