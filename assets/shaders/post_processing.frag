#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_texCoords;

uniform sampler2D scene, sceneDepth;
uniform vec2 resolution;
uniform float zoomEffect, zoom;

const float near = .1, far = 1000.;

void main()
{
    vec2 offset = v_texCoords * vec2(-2) + vec2(1);
    float r = texture(scene, v_texCoords + offset * .0010).r;
    float g = texture(scene, v_texCoords + offset * .0005).g;
    float b = texture(scene, v_texCoords).b;

    color = vec4(r, g, b, 1);

    vec2 mbOffset = vec2(gl_FragCoord.x / resolution.x, gl_FragCoord.y / resolution.y) - vec2(.5);

    if (zoomEffect > .05)
    {
        const int steps = 20;
        float div = 1.;

        for (int i = 0; i < steps; i++)
        {
            float effect = 1. - float(i) / float(steps);
            div += effect;
            color += texture(scene, v_texCoords - mbOffset * .09 * zoomEffect * (float(i) / float(steps))) * effect;
        }
        color /= div;
    }

    float depth = texture(sceneDepth, v_texCoords).r;
    depth = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    float fog = max(0., min(1., (depth * .24 - 12.) * .1));
    fog *= min(1., max(0., (zoom - .7) * 4.6));

    if (depth > 200. && color.r + color.g + color.b > 1.85) fog = 0.;

    // fog -= smoothstep(200., 500., depth);
    // if (depth > 200.) fog = 0.;

    color.rgb *= 1. - fog;
    color.rgb += vec3(.4, .6, .9) * fog;

    float vignette = smoothstep(3.0, .6, length(offset));
    color.rgb *= vignette;
}
