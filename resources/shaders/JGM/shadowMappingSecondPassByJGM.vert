#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

// uniforms
layout(set = 0, binding = 0) uniform UniformBufferObject 
{
  mat4 lightView;
  mat4 cameraView;
  mat4 proj;
} ubo;

// constants
layout( push_constant ) uniform ShadowMappingPushConstants
{
	mat4 modelMatrix;
	vec4 lightPosition;
} smpc;

layout( location = 0 ) out vec3 vertNormal;
layout( location = 1 ) out vec4 vertTexCoords;
layout( location = 2 ) out vec3 vertLight;

const mat4 bias = mat4(
	0.5, 0.0, 0.0, 0.0,	
	0.0, 0.5, 0.0, 0.0,	
 	0.0, 0.0, 1.0, 0.0,	
  	0.5, 0.5, 0.0, 1.0
);

mat4 lightModelView = ubo.lightView * smpc.modelMatrix; 
mat4 cameraModelView = ubo.cameraView * smpc.modelMatrix;

void main() {
	vec4 cameraViewSpacePosition = cameraModelView * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * cameraViewSpacePosition;
	
	vec3 cameraViewSpaceNormal = mat3( cameraModelView ) * inNormal;
	vertNormal = cameraViewSpaceNormal;
	
	vec4 lightViewSpacePosition = lightModelView * vec4(inPosition, 1.0);
	vertTexCoords = bias * ubo.proj * lightViewSpacePosition;
	
	vertLight = ( cameraModelView * vec4( smpc.lightPosition.xyz, 0.0 ) ).xyz;
}