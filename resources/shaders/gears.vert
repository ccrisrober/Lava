#version 450
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inNormal;

layout( binding = 0 ) uniform ubo
{
	mat4 projection;
	mat4 view;
	vec3 lightPos;
	vec3 viewPos;
};

layout(push_constant) uniform PushConstsVS
{
	mat4 model;
	vec3 meshColor;
};

layout( location = 0 ) out vec3 outPos;
layout( location = 1 ) out vec3 outNormal;

void main( )
{
	mat3 normalMatrix = mat3(transpose( inverse( model ) ) );
	outNormal = /*normalMatrix * */inNormal;

	outPos = vec3( model * vec4( inPos, 1.0 ) );
	gl_Position = projection * view * vec4( outPos, 1.0 );
}