#version 450
layout( location = 0 ) in vec3 pos;

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout( location = 0 ) out vec3 outUV;

void main( )
{
	gl_Position = proj * view * model * vec4( pos, 1.0 );
	vec3 wpos = vec3( model * vec4( pos, 1.0 ) );
	outUV = wpos * 0.5 + 0.5;
}