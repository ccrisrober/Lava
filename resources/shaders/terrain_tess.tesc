#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    float amount;
    float tess_level;
} ubo;
 
layout (vertices = 3) out;
 
layout (location = 0) in vec2 inUV[];
 
layout (location = 0) out vec2 outUV[3];
 
void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = ubo.tess_level;
		gl_TessLevelOuter[0] = ubo.tess_level;
		gl_TessLevelOuter[1] = ubo.tess_level;
		gl_TessLevelOuter[2] = ubo.tess_level;		
	}

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
	outUV[gl_InvocationID] = inUV[gl_InvocationID];
} 
