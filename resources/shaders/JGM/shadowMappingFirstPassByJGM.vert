#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout(location = 0) in vec3 inPosition;

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
} smpc;

mat4 lightModelView = ubo.lightView * smpc.modelMatrix; 

void main() {
	vec4 lightViewSpacePosition = lightModelView * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * lightViewSpacePosition;
}