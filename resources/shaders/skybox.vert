#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) out vec3 outUVW;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main( void ) 
{
    outUVW = inPos;
    // Truncate translation part of view matrix so skybox is always "around" the camera
    vec4 pos = ubo.proj * mat4(mat3(ubo.view)) * ubo.model * vec4(inPos, 1.0);
    gl_Position = pos.xyww; // Clip coords to NDC to ensure skybox is rendered at far plane
}
