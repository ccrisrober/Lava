#version 450

layout( location = 0 ) in int particleId;
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
};

void main()
{
	gl_Position = mvp * vec4( particles[particleId].pos, 1.0 );
	color = particles[particleId].color.rgb; //Pass color to fragment.
}