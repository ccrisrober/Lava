#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout(location = 0) in vec3 inPosition;
// normal
// texCoord

layout(location = 0) out vec3 fragColor;

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

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    // hardcoded color.
    fragColor = vec3(1.0, 0.0, 0.0);
}