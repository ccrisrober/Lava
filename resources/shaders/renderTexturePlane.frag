#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
  fragColor = texture(tex, uv);
}