#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec3 vLightWeighting;
layout (location = 1) in vec2 outUV;

layout (location = 0) out vec4 outColor;

const float alphaUniform = 0.75;

void main( )
{
    outColor = texture(texSampler, outUV);
    outColor.rgb *= vLightWeighting;
    outColor.a *= alphaUniform;
}