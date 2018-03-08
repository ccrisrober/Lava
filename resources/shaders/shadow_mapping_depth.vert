#version 450

layout(binding = 1) uniform ubo1
{
    mat4 lightSpaceMatrix;
};
layout(push_constant) uniform PushConsts
{
	mat4 model;
};
layout( location = 0 ) in vec3 position;

void main( )
{
    gl_Position = lightSpaceMatrix * model * vec4( position, 1.0 );
}