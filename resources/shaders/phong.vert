#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
//layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 lightPos;
} ubo;


layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
//layout (location = 2) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main( ) 
{
	outNormal = inNormal;
	//outUV = inUV;
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos, 1.0);
	
	outNormal = mat3(ubo.model) * inNormal;
	outPos = vec3(ubo.model * vec4(inPos, 1.0));
}