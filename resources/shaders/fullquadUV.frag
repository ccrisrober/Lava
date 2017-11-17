#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
  fragColor = vec4( uv.x, 1.0 - uv.y, 0.0, 1.0);
}