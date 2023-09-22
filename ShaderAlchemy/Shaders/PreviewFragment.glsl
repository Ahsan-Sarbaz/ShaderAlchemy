#version 330 core

out vec4 FinalColor;

in vec2 v_uv;

uniform sampler2D inputColorAttachment;

void main()
{
	FinalColor = texture(inputColorAttachment, v_uv);
}

