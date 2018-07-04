#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform sampler2D texSampler;

layout(early_fragment_tests) in;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
  vec4 color = texture( texSampler, uv );
	if( uv.x <= 0.5 )
  {
    if ( uv.y <= 0.5 )
    {
      fragColor = color * vec4( 1.0, 0.0, 0.0, 1.0 );
    }
    else
    {
      fragColor = color * vec4( 0.0, 1.0, 0.0, 1.0 );
    }
  }
  else
  {
    if ( uv.y <= 0.5 )
    {
      fragColor = color * vec4( 0.0, 0.0, 1.0, 1.0 );
    }
    else
    {
      fragColor = color * vec4( 1.0, 1.0, 1.0, 1.0 );
    }
  }
}