#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
    float up;
    float beta;
} ubo;


void main( void )
{
	const vec3 vel = vec3( ubo.up, 0.1, 0.0 );
	vec3 pos = inPosition + vel * ubo.time; // x = x0 + dt * v;
	gl_Position = vec4(pos, 1.0);
}