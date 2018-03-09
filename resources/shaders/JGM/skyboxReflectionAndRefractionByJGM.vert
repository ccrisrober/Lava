#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
 
// descriptor sets
layout( set = 0, binding = 0 ) uniform UniformBufferObject 
{
  mat4 view;
  mat4 proj;
} ubo;

// push constants
layout( push_constant ) uniform CubeMapRRPushConstants
{
  mat4 modelMatrix;
} cmrrpc;

// outputs
layout( location = 0 ) out vec3 vertPosition;
layout( location = 1 ) out vec3 vertNormal;
	
// auxiliar 	
mat4 modelView = ubo.view * cmrrpc.modelMatrix;	
	
void main()	{
  vertPosition = inPosition;
  vertNormal = inNormal;
  
  gl_Position = ubo.proj * modelView * vec4( inPosition, 1.0 );
}