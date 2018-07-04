#version 430

layout (input_attachment_index = 0, binding = 0) uniform subpassInput RenderTex;
layout( binding = 1 ) uniform sampler2D NoiseTex;

layout( binding = 2 ) uniform ubo0
{
  int width;
  int height;
  float radius;
};

layout( location = 0 ) in vec2 inUV;
layout( location = 0 ) out vec4 fragColor;

float luminance( vec3 color )
{
  return dot( color.rgb, vec3(0.2126, 0.7152, 0.0722) );
}

void main( )
{
  vec4 noise = texture(NoiseTex, inUV);
  vec3 color = subpassLoad(RenderTex).rgb;
  float green = luminance( color );

  float dist1 = length(gl_FragCoord.xy - vec2(width/4.0, height/2.0));
  float dist2 = length(gl_FragCoord.xy - vec2(3.0 * width/4.0, height/2.0));
  if( dist1 > radius && dist2 > radius ) green = 0.0;

  fragColor = vec4(0.0, green * clamp( noise.a, 0.0, 1.0) , 0.0 ,1.0);
}