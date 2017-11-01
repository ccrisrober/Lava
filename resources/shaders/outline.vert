#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(push_constant) uniform PushConsts
{
    float outlineWidth;
} pushConsts;

void main( )
{
	// Extrude along normal
	vec4 pos = vec4(inPosition + inNormal * pushConsts.outlineWidth, 1.0);
	gl_Position = ubo.proj * ubo.view * ubo.model * pos;
}