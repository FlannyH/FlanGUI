#version 330 core
precision mediump float;

out vec4 frag_color;
in vec2 texcoord;
in vec4 colour;

uniform sampler2D tex;

void main()
{
	frag_color = colour;
}