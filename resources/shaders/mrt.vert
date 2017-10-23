#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec2 inUV;

layout( binding = 0 ) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 instancePos[ 9 ];
} ubo;

layout( location = 0 ) out vec3 outPos;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main( )
{
	vec3 tmpPos = inPos + ubo.instancePos[ gl_InstanceIndex ];

	gl_Position = ubo.proj * ubo.view * ubo.model * vec4( tmpPos, 1.0 );

	outUV = inUV;

    mat3 normalMatrix = mat3(transpose(inverse(ubo.view * ubo.model)));
    outNormal = normalMatrix * inNormal;
}