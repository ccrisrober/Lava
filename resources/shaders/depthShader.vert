#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
	fragNormal = (transpose(inverse(ubo.view * ubo.model)) * vec4(vertNormal, 1.0)).xyz;
	fragPos = (ubo.view * ubo.model * vec4(vertPosition, 1.0)).xyz;
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vertPosition, 1.0);
}