#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 projection;
  vec3 viewPos;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out float texLevel;

out gl_PerVertex
{
  vec4 gl_Position;
};

void main( )
{
  gl_Position = view * model * vec4(inPosition, 1.0);
  texLevel = float( gl_VertexIndex % 4 );
}