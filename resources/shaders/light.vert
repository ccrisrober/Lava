#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	mat4 modelInstance[ 4 ];
	vec3 colorInstance[ 4 ];
} ubo;

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 color;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.proj * ubo.view * 
    	ubo.modelInstance[gl_InstanceIndex] * vec4(aPos, 1.0);

    color = ubo.colorInstance[ gl_InstanceIndex ];
}