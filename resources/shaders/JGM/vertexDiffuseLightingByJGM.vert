#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
// texCoord
// color

layout(location = 0) out float fragColor;

// uniforms
layout(set = 0, binding = 0) uniform UniformBufferObject 
{
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

vec3 lightPosition = vec3( 1.0, 2.0, 0.0 );

void main() {
	mat4 modelView = ubo.view * ubo.model;
    gl_Position = ubo.proj * modelView * vec4(inPosition, 1.0);
    vec3 normal = mat3( modelView ) * inNormal;
    // vertex normal as color
    fragColor = max( 0.0, dot( normal, lightPosition ) ) + 0.1;
}