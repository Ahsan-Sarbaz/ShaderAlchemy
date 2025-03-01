#version 450 core

out vec4 FinalColor;

in vec2 v_uv;

layout (binding = 0) uniform sampler2D iLastPass;

void main()
{
	FinalColor = texture(iLastPass, v_uv);
}

