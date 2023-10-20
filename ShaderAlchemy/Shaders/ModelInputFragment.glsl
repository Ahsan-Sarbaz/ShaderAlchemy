#version 450 core

out vec4 FinalColor;

in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec2 v_uv;

uniform vec3	iEyePosition;			// camera position
uniform vec3	iResolution;			// viewport resolution (in pixels)
uniform float	iTime;					// shader playback time (in seconds)
uniform float	iTimeDelta;				// render time (in seconds)
uniform float	iFrameRate;				// shader frame rate
uniform int		iFrame;					// shader playback frame
uniform float	iChannelTime[16];		// channel playback time (in seconds)
uniform vec3	iChannelResolution[16];	// channel resolution (in pixels)
uniform vec4	iMouse;					// mouse pixel coords. xy: current (if MLB down), zw: click

layout (binding = 0) uniform sampler2D Diffuse;			// Diffuse / Base Color
layout (binding = 1) uniform sampler2D SpecularMap;		// Specular / Metallness
layout (binding = 2) uniform sampler2D NormalMap;		// Normals / Normals Camera
layout (binding = 3) uniform sampler2D Shininess;		// Shininess / Diffuse Roughness
layout (binding = 4) uniform sampler2D LightMap;		// LightMap / Ambient Occlusion
layout (binding = 5) uniform sampler2D Emissive;		// Emissive / Emission Color
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
	vec3 diffuse = texture(Diffuse, v_uv).rgb;
	
	FinalColor = vec4(diffuse, 1.0);
}

