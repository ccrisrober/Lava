#version 450

layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor;

layout( location = 0 ) out vec3 outColor;

layout( push_constant ) uniform PushConsts
{
	mat4 MVP;
};

void main( ) 
{
	gl_Position = MVP * vec4( inPos.xyz, 1.0 );
	outColor = inColor;
}
