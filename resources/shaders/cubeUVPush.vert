#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(push_constant) uniform PushConsts
{
	vec3 positionOffset;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    gl_Position.xyz += positionOffset;
    outUV = inUV;
}