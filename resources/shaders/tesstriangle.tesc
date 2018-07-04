#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( vertices = 3 ) out;

#define ID gl_InvocationID

void main( )
{
	if( ID == 0 )
	{
		gl_TessLevelInner[ 0 ] = 3.0;
		gl_TessLevelOuter[ 0 ] = 3.0;
		gl_TessLevelOuter[ 1 ] = 8.0;
		gl_TessLevelOuter[ 2 ] = 10.0;
	}
	gl_out[ ID ].gl_Position = gl_in[ ID ].gl_Position;
}