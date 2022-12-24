#version 330 core
precision mediump float;

out vec4 frag_color;
in vec2 texcoord;
in vec4 colour;
in vec4 clip_rect;

uniform sampler2D tex;
uniform ivec2 resolution;

void main()
{
    if (gl_FragCoord.x < clip_rect.x)
        discard;
    else if (gl_FragCoord.x > clip_rect.z)
        discard;
    else if ((resolution.y - gl_FragCoord.y) < clip_rect.y)
        discard;
    else if ((resolution.y - gl_FragCoord.y) > clip_rect.w)
        discard;
    if (colour.a != 0.0f)
	    frag_color = colour;
    else {
        if (texture(tex, texcoord).a == 0) discard;
        frag_color = colour * texture(tex, texcoord);
    }
}