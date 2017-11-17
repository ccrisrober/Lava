#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 vLightWeighting;
layout(location = 1) out vec2 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

const vec3 ambientColor = vec3( 0.2, 0.2, 0.2 );
const vec3 directionalColor = vec3( 0.8, 0.8, 0.8 );
const vec3 lightingDirection = vec3( -0.25, -0.25, -1.0 );

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    mat3 normalMatrix = mat3(inverse(transpose(ubo.view * ubo.model)));
    vec3 transformedNormal = normalize(normalMatrix * inNormal);

    float directionalLightWeighting = max(dot(transformedNormal, lightingDirection), 0.0);
    vLightWeighting = ambientColor + directionalColor * directionalLightWeighting;

    outUV = inUV;
}