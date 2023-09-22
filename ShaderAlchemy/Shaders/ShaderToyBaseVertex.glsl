#version 450 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

out vec2 iScreenQuadUV;

void main()
{
	iScreenQuadUV = uv;
	gl_Position = position;
}