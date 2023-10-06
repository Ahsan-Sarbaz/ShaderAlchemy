#version 450 core

out vec4 FinalColor;

in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec2 v_uv;

void main()
{
	FinalColor = vec4(v_normal, 1.0);
}

