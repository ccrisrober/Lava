#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 ClipPlane;
};

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 FragPos;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_ClipDistance[ ];
};

void main( )
{
	FragPos = aPos;
    gl_Position = proj * view * model * vec4(FragPos, 1.0);

    gl_ClipDistance[0] = dot(model * vec4(aPos, 1.0), ClipPlane);
}