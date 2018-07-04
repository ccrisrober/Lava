#version 450

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

void main( )
{
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    outNormal = mat3(transpose(inverse(view * model))) * inNormal;
}