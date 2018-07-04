#version 450

layout(binding = 0) uniform ubo0
{
    mat4 modelViewMatrix;
    mat4 projectionMatrix;
    float sineTime;
};

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 offset;
layout( location = 2 ) in vec4 color;
layout( location = 3 ) in vec4 orientationStart;
layout( location = 4 ) in vec4 orientationEnd;

layout( location = 0 ) out vec3 vPosition;
layout( location = 1 ) out vec4 vColor;

void main( )
{

	vPosition = offset * max( abs( sineTime * 2.0 + 1.0 ), 0.5 ) + position;
	vec4 orientation = normalize( mix( orientationStart, orientationEnd, sineTime ) );
	vec3 vcV = cross( orientation.xyz, vPosition );
	vPosition = vcV * ( 2.0 * orientation.w ) + ( cross( orientation.xyz, vcV ) * 2.0 + vPosition );

	vColor = color;

	gl_Position = projectionMatrix * modelViewMatrix * vec4( vPosition, 1.0 );
}
