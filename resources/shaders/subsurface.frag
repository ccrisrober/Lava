#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 outNormal;
//layout (location = 1) out vec2 outTexCoord;

layout (location = 0) out vec4 outColor;

void main( )
{
  outColor = vec4(outNormal, 1.0);
  //outColor.rg += outTexCoord;
  //outColor.rg -= outTexCoord;
}