#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 instancePos[ 9 ];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
	vec3 tmpPos = inPosition;

	outPosition = vec3( ubo.view * ubo.model * vec4( tmpPos, 1.0 ) );
	outPosition += ubo.instancePos[ gl_InstanceIndex ];
    gl_Position = ubo.proj * vec4( outPosition, 1.0 );
    outNormal = inNormal;
}