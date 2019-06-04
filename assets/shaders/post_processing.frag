#version 430 core
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

    float vignette = smoothstep(3.0, .6, length(offset));
    color.rgb *= vignette;
}
