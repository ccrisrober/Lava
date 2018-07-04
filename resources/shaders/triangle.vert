#version 430

layout( location = 0 ) in vec2 inVertex;
layout( location = 1 ) in vec3 inColor;

layout( location = 0 ) out vec3 outColor;

void main( )
{
  outColor = inColor;
  gl_Position = vec4( inVertex, 0.0, 1.0 );
}