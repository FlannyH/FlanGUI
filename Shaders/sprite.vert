#version 330 core
precision mediump float;
layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_texcoord;
layout (location = 2) in vec4 i_colour;
out vec2 texcoord;
out vec4 colour;

void main()
{
	gl_Position = vec4(i_position, 1);
	texcoord = i_texcoord;
	colour = i_colour;
}