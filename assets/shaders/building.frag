#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;
in float light;

uniform sampler2D buildingTexture;

void main()
{
    color = texture(buildingTexture, v_uv);
    color.rgb *= light;
}
