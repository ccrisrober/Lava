#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec2 inPosition;
layout( location = 1 ) in vec3 color;
layout( location = 2 ) in uint sides;

layout ( location = 0 ) out vec3 vColor;
layout ( location = 1 ) out uint vSides;

out gl_PerVertex
{
  vec4 gl_Position;
};

layout(push_constant) uniform PushConsts
{
  float time;
};

vec2 rotate( vec2 v, float a )
{
  float s = sin( a );
  float c = cos( a );
  mat2 m = mat2( c, -s, s, c );
  return m * v;
}

void main( )
{
  gl_Position = vec4(inPosition + ( rotate( inPosition, time ) ) * 0.5, 0.0, 1.0);
  vColor = color;
  vSides = sides;
}