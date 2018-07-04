#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 fragColor;

void main() 
{
	fragColor = vec4( inUVW, 1.0 );
	fragColor = texture(samplerCubeMap, inUVW);
}