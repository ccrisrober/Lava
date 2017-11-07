#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


layout(binding = 1) uniform UBO2
{
	float outlineWidth;
} ubo2;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	// Extrude along normal
	vec4 pos = vec4(inPosition + inNormal * ubo2.outlineWidth, 1.0);
	gl_Position = ubo.proj * ubo.view * ubo.model * pos;
}