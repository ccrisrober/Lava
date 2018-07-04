#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D texSampler;

void main( void )
{
  fragColor = texture(texSampler, uv);
  fragColor.rgb *= color;
}