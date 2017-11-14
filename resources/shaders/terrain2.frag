#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( location = 0 ) in vec3 geom_normal;
layout( location = 1 ) in float geom_height;
layout( location = 0 ) out vec4 frag_color;

const vec4 green = vec4( 0.2, 0.5, 0.1, 1.0 );
const vec4 brown = vec4( 0.6, 0.5, 0.3, 1.0 );
const vec4 white = vec4( 1.0 );

void main( )
{
	vec4 color = mix( green, brown, smoothstep( 0.0, 0.4, geom_height ) );
	color = mix( color, white, smoothstep( 0.6, 0.9, geom_height ) );
	float diffuse_light = max( 0.0, dot( geom_normal, vec3( 0.58, 0.58, 0.58 ) ) );
	frag_color = vec4( 0.05, 0.05, 0.0, 0.0 ) + diffuse_light * color;
}
