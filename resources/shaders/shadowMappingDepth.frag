#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 fragColor;

const vec3 depthColor = vec3( 0.0, 0.0, 0.0 );

void main(void)
{
	fragColor = vec4( depthColor, 1.0 );
}