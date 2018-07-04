#version 430

layout(push_constant) uniform PushConsts
{
	vec3 color;
};

layout( location = 0 ) out vec4 fragColor;

void main( )
{
  fragColor = vec4( color, 1.0 );
}