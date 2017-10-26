#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	mat4 modelInstance[ 10 ];
} ubo;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec2 TexCoords;

out gl_PerVertex {
    vec4 gl_Position;
};

void main( )
{
    FragPos = vec3(ubo.modelInstance[gl_InstanceIndex] * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(ubo.modelInstance[gl_InstanceIndex]))) * aNormal;  
    TexCoords = aTexCoords;
    
    gl_Position = ubo.proj * ubo.view * vec4(FragPos, 1.0);
}