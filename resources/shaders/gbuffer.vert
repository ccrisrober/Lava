#version 450

layout(binding = 0) uniform ubo0
{
    mat4 proj;
    mat4 view;
};

layout(push_constant) uniform PushConsts
{
	mat4 model;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec2 UV;

void main( )
{
    FragPos = vec3(model * vec4(inPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * inNormal;  

    UV = inTexCoord;
    
    gl_Position = proj * view * vec4(FragPos, 1.0);
}