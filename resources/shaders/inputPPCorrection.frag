#version 450
layout( input_attachment_index = 0, set = 0, binding = 0 )

uniform subpassInput InputAttachment;
layout( location = 0 ) out vec4 fragColor;

void main( )
{
	vec4 color = subpassLoad( InputAttachment );
	float grey = dot( color.rgb, vec3( 0.2, 0.7, 0.1 ) );
	fragColor = grey * vec4( 1.5, 1.0, 0.5, 1.0 );
}
