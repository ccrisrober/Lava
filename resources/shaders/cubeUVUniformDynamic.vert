#version 450

layout( set = 0, binding = 0 ) uniform ubo0
{
    mat4 model;
};

layout( set = 0, binding = 1 ) uniform ubo1
{
    mat4 proj;
    mat4 view;
};

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec2 inUV;

layout( location = 0 ) out vec2 outUV;

void main( )
{
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    outUV = inUV;
}