#version 450

layout( binding = 0 ) uniform ubo0
{
    mat4 view;
    mat4 proj;
};

layout( push_constant ) uniform PushConsts
{
    mat4 model;
};

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec2 inUV;

layout( location = 0 ) out vec2 outUV;

void main( )
{
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    outUV = inUV;
}