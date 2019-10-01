#version 300 es
precision mediump float;

out vec4 color;

in vec2 v_uv;

uniform sampler2D shipTexture;

void main()
{
    color = texture(shipTexture, v_uv);
}
