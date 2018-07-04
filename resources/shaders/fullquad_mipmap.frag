#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
//  fragColor = textureLod(tex, uv, 0.0);
//  fragColor = vec4( uv, 0.0, 1.0 );

  if( uv.x <= 0.5 )
  {
    if ( uv.y <= 0.5 )
    {
      fragColor = textureLod(tex, uv, 0.0);
    }
    else
    {
      fragColor = textureLod(tex, uv, 1.0);
    }
  }
  else
  {
    if ( uv.y <= 0.5 )
    {
      fragColor = textureLod(tex, uv, 2.0);
    }
    else
    {
      fragColor = textureLod(tex, uv, 3.0);
    }
  }
}