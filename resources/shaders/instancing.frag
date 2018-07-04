#version 450

layout(binding = 1) uniform ubo1
{
    float time;
};

layout( location = 0 ) in vec3 vPosition;
layout( location = 1 ) in vec4 vColor;
layout( location = 0 ) out vec4 fragColor;

void main( )
{
	vec4 color = vec4( vColor );
	color.r += sin( vPosition.x * 10.0 + time ) * 0.5;
	fragColor = color;
}