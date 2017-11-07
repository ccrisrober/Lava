#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    mat3 normalMatrix = mat3(inverse(transpose(ubo.view * ubo.model)));
    outNormal = normalize(normalMatrix * inNormal);
}