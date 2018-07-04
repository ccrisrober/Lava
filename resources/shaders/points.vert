#version 450

layout( binding = 0 ) uniform ubo0
{
  mat4 modelViewProjection;
};

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inColor;

layout( location = 0 ) out vec3 outColor;

void main( )
{
  gl_Position = modelViewProjection * vec4( inPosition, 1.0 );
  outColor = inColor;
  gl_PointSize = 15.0;
}