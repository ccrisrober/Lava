#version 450

layout( binding = 1 ) uniform sampler2D grassTex;

layout( location = 0 ) in vec2 uv;
layout( location = 0 ) out vec4 fragColor;

layout( binding = 2 ) uniform ubo2
{
	float fAlphaTest;
	float fAlphaMultiplier;
};

void main( )
{
	vec4 vTexColor = texture( grassTex, uv );
	float fNewAlpha = vTexColor.a * fAlphaMultiplier;
	if( fNewAlpha < fAlphaTest ) discard;
	if( fNewAlpha > 1.0 )
	{
		fNewAlpha = 1.0;
	}
	fragColor = vec4( vTexColor.xyz, fNewAlpha );
}