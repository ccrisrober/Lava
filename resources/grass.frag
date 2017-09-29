#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D grassTex;

layout (location = 0) in vec2 TexCoord;
layout (location = 0) out vec4 fragColor;

const float fAlphaTest = 0.25;
const float fAlphaMultiplier = 1.5;

void main( void )
{
	vec4 vTexColor = texture(grassTex, TexCoord);
	float fNewAlpha = vTexColor.a * fAlphaMultiplier;
	if(fNewAlpha < fAlphaTest) discard;
	if(fNewAlpha > 1.0f)
	fNewAlpha = 1.0f;
	fragColor = vec4(vTexColor.xyz, fNewAlpha);
}