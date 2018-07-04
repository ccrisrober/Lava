#version 450

layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor;

layout( binding = 0 ) uniform ubo0 
{
	mat4 projection;
	mat4 view;
};

layout( binding = 1 ) uniform ubo1 
{
	mat4 model; 
};

layout( location = 0 ) out vec3 outColor;

void main( ) 
{
	outColor = inColor;
	mat4 modelView = view * model;
	vec3 worldPos = vec3(modelView * vec4(inPos, 1.0));
	gl_Position = projection * modelView * vec4(inPos.xyz, 1.0);
}