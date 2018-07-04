#version 450

/* 
// http://www.nvidia.com/object/Projective_Texture_Mapping.html
vec3 projPos = vec3(2.0f,5.0f,5.0f);
vec3 projAt = vec3(-2.0f,-4.0f,0.0f);
vec3 projUp = vec3(0.0f,1.0f,0.0f);
mat4 projView = glm::lookAt(projPos, projAt, projUp);
mat4 projProj = glm::perspective(glm::radians(30.0f), 1.0f, 0.2f, 1000.0f);
mat4 projScaleTrans = glm::translate(mat4(), vec3(0.5f)) * glm::scale(mat4(), vec3(0.5f));
prog.setUniform("ProjectorMatrix", projScaleTrans * projProj * projView);
*/

layout( location = 0 ) in vec3 VertexPosition;
layout( location = 1 ) in vec3 VertexNormal;

layout( set = 0, binding = 0 ) uniform ubo00
{
	mat4 view;
	mat4 projection;	
};

layout(push_constant) uniform PushConsts
{
	mat4 model;
};

layout( set = 1, binding = 0 ) uniform ubo10
{
	mat4 projector;	
};

layout( location = 0 ) out vec3 EyeNormal; // Normal in eye coordinates
layout( location = 1 ) out vec4 EyePosition; // Position in eye coordinates
layout( location = 2 ) out vec4 ProjTexCoord;

void main( )
{
	mat4 ModelViewMatrix = view * model;
	mat3 NormalMatrix = mat3(inverse(transpose( ModelViewMatrix ) ) );
	vec4 pos4 = vec4(VertexPosition,1.0);
	EyeNormal = normalize(NormalMatrix * VertexNormal);
	EyePosition = ModelViewMatrix * pos4;
	ProjTexCoord = projector * (model * pos4);
	gl_Position = projection * ModelViewMatrix * pos4;
}