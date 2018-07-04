#version 450
layout( location = 0 ) in flat vec3 Normal;

layout( location = 0 ) out vec4 fragColor;

void main( void )
{
  fragColor = vec4(Normal, 1.0);
}