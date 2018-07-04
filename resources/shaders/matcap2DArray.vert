#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
    float level;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    mat3 normalMatrix = mat3(inverse(transpose(view * model)));
    outNormal = normalize(normalMatrix * inNormal);
}