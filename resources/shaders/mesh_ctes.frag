#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 outNormal;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int MODEL = 0;

void main( void )
{
  outColor = vec4(outNormal, 1.0);

  vec4 color;
  switch( MODEL )
  {
  case 0: // RED
    color = vec4( 1.0, 0.0, 0.0, 1.0 );
    break;
  case 1: // GREEN
    color = vec4( 0.0, 1.0, 0.0, 1.0 );
    break;
  case 2: // BLUE
    color = vec4( 0.0, 0.0, 1.0, 1.0 );
    break;
  }

  outColor = mix( outColor, color, 0.5 );
}