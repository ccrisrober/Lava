#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

// Instance attributes
layout(location = 2) in vec3 instancePos;

layout(location = 0) out vec2 outUV;

/*out gl_PerVertex {
    vec4 gl_Position;
};*/

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition + instancePos, 1.0);
    outUV = inUV;
}