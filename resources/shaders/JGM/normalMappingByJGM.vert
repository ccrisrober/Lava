#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec2 inTexCoord;
layout( location = 3 ) in vec3 inTangent;
layout( location = 4 ) in vec3 inBitangent;  

// descriptor sets
layout( set = 0, binding = 0 ) uniform UniformBufferObject 
{
  mat4 view;
  mat4 proj;
} ubo;

// push constants
layout( push_constant ) uniform NormalMappingPushConstants
{
  mat4 modelMatrix;
} nmpc;

// outputs
layout( location = 0 ) out vec3 vertPosition;
layout( location = 1 ) out vec3 vertNormal;
layout( location = 2 ) out vec2 vertTexCoord;
layout( location = 3 ) out vec3 vertTangent;
layout( location = 4 ) out vec3 vertBitangent;
	
// auxiliar 	
mat4 modelView = ubo.view * nmpc.modelMatrix;	
	
void main()	{
  vec4 viewSpacePosition = modelView * vec4( inPosition, 1.0 );
  gl_Position = ubo.proj * viewSpacePosition;
  
  vertPosition = viewSpacePosition.xyz;
  vertNormal = mat3( modelView ) * inNormal;
  vertTexCoord = inTexCoord;
  vertTangent = mat3( modelView ) * inTangent;
  vertBitangent = mat3( modelView ) * inBitangent;
}