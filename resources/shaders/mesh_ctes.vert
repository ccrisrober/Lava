#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 Normal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    FragPos = vec3(model * vec4(inPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * inNormal;  
    
    gl_Position = proj * view * vec4(FragPos, 1.0);
}