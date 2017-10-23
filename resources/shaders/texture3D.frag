#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler3D samplerColor;

layout (location = 0) in vec3 inUV;

layout (location = 0) out vec4 fragColor;

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() 
{
	float color = texture(samplerColor, inUV).r;

	fragColor = vec4(hsv2rgb( vec3( color, 1.0, 1.0 ) ), 1.0);	
}