#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec2 inUV;

layout( location = 0 ) out vec3 outPosition;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec2 outUV;

void main() {
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    outNormal = mat3(transpose(inverse(model))) * inNormal;
    outPosition = vec3(model * vec4(inPosition, 1.0));
    outUV = inUV;
}