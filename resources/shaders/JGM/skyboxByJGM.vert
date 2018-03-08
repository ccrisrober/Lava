#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout( location = 0 ) in vec3 inPosition;
 
// descriptor sets
layout( set = 0, binding = 0 ) uniform UniformBufferObject 
{
  mat4 view;
  mat4 proj;
} ubo;

// push constants
layout( push_constant ) uniform SkyboxPushConstants
{
  mat4 modelMatrix;
} spc;

// outputs
layout( location = 0 ) out vec3 vertTexCoord;
	
// auxiliar 	
mat4 modelView = ubo.view * spc.modelMatrix;	
	
void main()	{
  vertTexCoord = inPosition;  
 
  // Only modelView rotation component.	
  vec3 position = mat3( modelView ) * inPosition;
  // The vertex will lie on the far clipping plane.
  gl_Position = ( ubo.proj * vec4( position, 0.0 ) ).xyzz;
}