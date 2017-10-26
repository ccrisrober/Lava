#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 outPos;
layout (location = 1) in vec3 outNormal;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 position;

void main( )
{
  color = vec4(outNormal, 1.0);
  position = vec4(outPos, 1.0);
}