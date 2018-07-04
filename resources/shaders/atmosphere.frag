#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 1 ) uniform sampler2D sColorMap;

layout( location = 1 ) in vec2 TexCoord;

layout( location = 0 ) out vec4 fragColor;

void main( void )
{
  fragColor = texture( sColorMap, TexCoord );
}