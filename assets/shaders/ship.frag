#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;
in float light;

uniform sampler2D shipTexture;

void main()
{
    color = vec4(texture(shipTexture, v_uv).rgb * light, 1);
}
