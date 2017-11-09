#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 inVertex;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 TexCoord;

void main()
{
  gl_Position = inVertex;
  TexCoord = inTexCoord;
}