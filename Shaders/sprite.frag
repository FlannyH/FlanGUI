#version 330 core
precision mediump float;

out vec4 frag_color;
in vec2 texcoord;
in vec4 colour;

uniform sampler2D tex;

void main()
{
    if (colour.a != 0.0f)
	    frag_color = colour;
    else {
        if (texture(tex, texcoord).a == 0) discard;
        frag_color = colour * texture(tex, texcoord);
    }
}