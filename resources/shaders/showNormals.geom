#version 450

layout( binding = 0 ) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 proj;
};

layout( triangles ) in;
layout( line_strip, max_vertices = 6 ) out;

layout( location = 0 ) in vec3 inNormal[ ];

layout( location = 0 ) out vec3 color;

const float MAGNITUDE = 0.1;

void GenerateLine( int index )
{
	vec3 pos = gl_in[ index ].gl_Position.xyz;
	vec3 normal = inNormal[ index ].xyz;

	gl_Position = proj * view * (model * vec4(pos, 1.0));
	color = vec3(1.0, 0.0, 0.0);
	EmitVertex();

	gl_Position = proj * view * (model * vec4(pos + normal * MAGNITUDE, 1.0));
	color = vec3(0.0, 0.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

void main( void )
{
	GenerateLine( 0 ); // first vertex normal
	GenerateLine( 1 ); // second vertex normal
	GenerateLine( 2 ); // third vertex normal
}