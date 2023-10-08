#version 450 core

out vec4 FinalColor;

in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec2 v_uv;

layout (binding = 0) uniform sampler2D iChannel0;
layout (binding = 1) uniform sampler2D iChannel1;
layout (binding = 2) uniform sampler2D iChannel2;
layout (binding = 3) uniform sampler2D iChannel3;
layout (binding = 4) uniform sampler2D iChannel4;
layout (binding = 5) uniform sampler2D iChannel5;
layout (binding = 6) uniform sampler2D iChannel6;
layout (binding = 7) uniform sampler2D iChannel7;
layout (binding = 8) uniform sampler2D iChannel8;
layout (binding = 9) uniform sampler2D iChannel9;
layout (binding = 10) uniform sampler2D iChannel10;
layout (binding = 11) uniform sampler2D iChannel11;
layout (binding = 12) uniform sampler2D iChannel12;
layout (binding = 13) uniform sampler2D iChannel13;
layout (binding = 14) uniform sampler2D iChannel14;
layout (binding = 15) uniform sampler2D iChannel15;

void main()
{
	FinalColor = vec4(v_normal, 1.0);
}

