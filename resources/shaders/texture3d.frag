#version 450

layout( binding = 1 ) uniform sampler3D texSampler;

layout( location = 0 ) in vec3 inUV;
layout( location = 0 ) out vec4 fragColor;

void main( )
{
	fragColor = texture( texSampler, inUV );
	//fragColor = vec4( inUV, 1.0 );
}