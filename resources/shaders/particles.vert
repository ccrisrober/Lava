#version 430

layout (location = 0) in vec4 aPosition;

layout(binding = 0) uniform ubo0
{
	mat4 mvp;
};

void main( )
{
    gl_Position = mvp * aPosition;
}