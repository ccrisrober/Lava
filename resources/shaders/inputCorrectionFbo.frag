#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(early_fragment_tests) in;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
	vec4 color = texture( texSampler, uv );
  float grey = dot( color.rgb, vec3( 0.2, 0.7, 0.1 ) );
  fragColor = grey * vec4( 1.5, 1.0, 0.5, 1.0 );
}