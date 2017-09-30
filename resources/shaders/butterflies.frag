#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 fragColor;

void main( void )
{
	vec4 tex = texture(texSampler, TexCoord).rgba;
	if(tex.a < 0.4) discard;
	fragColor = vec4(tex);
}