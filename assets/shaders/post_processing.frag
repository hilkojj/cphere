#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_texCoords;

uniform sampler2D scene;

void main()
{
    vec2 offset = v_texCoords * vec2(-2) + vec2(1);
    float r = texture(scene, v_texCoords + offset * .0010).r;
    float g = texture(scene, v_texCoords + offset * .0005).g;
    float b = texture(scene, v_texCoords).b;

    color = vec4(r, g, b, 1);

    // const vec2 resolution = vec2(1600, 900);

    // vec2 mbOffset = vec2(gl_FragCoord.x / resolution.x, gl_FragCoord.y / resolution.y) - vec2(.5);

    // const int steps = 20;

    // float div = 1.;

    // for (int i = 0; i < steps; i++)
    // {
    //     float effect = 3. - float(i) / float(steps);
    //     div += effect;
    //     color += texture(scene, v_texCoords - mbOffset * .2 * (float(i) / float(steps))) * effect;
    // }

    // color /= div;

    float vignette = smoothstep(3.0, .6, length(offset));
    color.rgb *= vignette;
}
