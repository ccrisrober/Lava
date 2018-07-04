#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
};


layout(binding = 1) uniform ubo1
{
	float outlineWidth;
};

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;

void main( )
{
	// Extrude along normal
	vec4 pos = vec4(inPosition + inNormal * outlineWidth, 1.0);
	gl_Position = proj * view * model * pos;
}