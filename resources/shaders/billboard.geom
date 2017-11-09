#version 450
layout( points ) in;
layout( set = 0, binding = 0) uniform UniformBuffer
{
	mat4 model;
	mat4 view;
	mat4 proj;
};

layout( triangle_strip, max_vertices = 4 ) out;
layout( location = 0 ) out vec2 uv;

const float SIZE = 0.1;

void main( )
{
	vec4 position = gl_in[0].gl_Position;

	gl_Position = proj * (position + vec4( -SIZE, SIZE, 0.0, 0.0 ) );
	uv = vec2( -1.0, 1.0 );
	EmitVertex( );

	gl_Position = proj * (position + vec4( -SIZE, -SIZE, 0.0, 0.0 ) );
	uv = vec2( -1.0, -1.0 );
	EmitVertex( );

	gl_Position = proj * (position + vec4( SIZE, SIZE, 0.0, 0.0 ) );
	uv = vec2( 1.0, 1.0 );
	EmitVertex( );

	gl_Position = proj * (position + vec4( SIZE, -SIZE, 0.0, 0.0 ) );
	uv = vec2( 1.0, -1.0 );
	EmitVertex( );

	EmitPrimitive( ); 
}