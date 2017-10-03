#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;

layout(push_constant) uniform PushConsts
{
    vec2 scale;
    vec2 translate;
} pushConstants;

void main( void )
{
    outUV = inUV;
    gl_Position = vec4(inPos * pushConstants.scale + 
        pushConstants.translate, 0.0, 1.0);
}