#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 1 ) uniform sampler2DArray texSampler;

layout ( location = 0 ) in vec2 inUV;
layout ( location = 1 ) in float inLevel;

layout ( location = 0 ) out vec4 outColor;

void main( )
{
  vec2 uv = inUV;
  uv.y = 1.0 - uv.y;
  outColor = texture( texSampler, vec3( uv, inLevel ) );
  if(outColor.a < 0.75) 
  {
  	discard;
  }
	else
	{
		outColor.a = 0.5;
	}

}