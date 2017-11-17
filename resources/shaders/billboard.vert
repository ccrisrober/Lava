#version 450
layout(location = 0) in vec3 position;
layout( set = 0, binding = 0) uniform UniformBuffer
{
	mat4 model;
	mat4 view;
	mat4 proj;
};

void main( )
{
	gl_Position = view * model * vec4( position, 1.0 );
}