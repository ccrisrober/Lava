#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( triangles, equal_spacing, ccw ) in;

void main( )
{
	gl_Position = 
		gl_in[ 0 ].gl_Position * gl_TessCoord.x + 
		gl_in[ 1 ].gl_Position * gl_TessCoord.y + 
		gl_in[ 2 ].gl_Position * gl_TessCoord.z;
}