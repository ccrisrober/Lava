#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 outNormal;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushConsts
{
  vec4 color;
} pushConsts;

void main( void )
{
  outColor = vec4(outNormal, 1.0);

  outColor = mix( outColor, pushConsts.color, 0.75 );
  //outColor = pushConsts.color;
}