#version 450

#define particleId gl_VertexIndex

//layout( location = 0 ) in int particleId;
layout( location = 0 ) out vec3 color;

struct Particle
{
	vec3	pos;
	vec3	ppos;
	vec3	home;
	vec4	color;
	float	damping;
};

//Particle buffer
layout( std140, binding = 0 ) buffer Part
{
    Particle particles[];
};

//Incoming model view matrix
layout(binding = 2) uniform UBO
{
	mat4 mvp;
	float pointSize;
};

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main( )
{
	/*gl_Position = mvp * vec4( particles[particleId].pos, 1.0 );
	gl_Position = mvp * vec4( 0.5, 0.5, 0.5, 1.0 );
	color = particles[particleId].color.rgb; //Pass color to fragment.
	color = vec3( 0.5, 0.5, 0.5 );
	gl_PointSize = 25.0;*/


	gl_PointSize = 8.0;
	color = particles[particleId].color.rgb; //Pass color to fragment.
	gl_Position =  mvp * vec4(particles[particleId].pos, 1.0);
}