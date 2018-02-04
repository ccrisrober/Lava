#version 440

layout( location = 0 ) out vec4 fragColor;

layout( std140, set = 0, binding = 0 ) uniform ubo
{
	vec4 iMouse;
	vec4 iDate;
	vec4 iResolution;
	vec4 iChannelTime;
	vec4 iChannelResolution[ 4 ];

	float iGlobalDelta;
	float iGlobalFrame;
	float iGlobalTime;
	float iSampleRate;
};

layout( set = 1, binding = 0 ) uniform sampler2D iChannel0;
layout( set = 1, binding = 1 ) uniform sampler2D iChannel1;
layout( set = 1, binding = 2 ) uniform sampler2D iChannel2;
layout( set = 1, binding = 3 ) uniform sampler2D iChannel3;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	const vec2 middle = iResolution.xy * 0.5;
	vec2 uv = vec2( fragCoord.xy / iResolution.xy );
	uv.y = 1.0 - uv.y;

	fragColor.rgb = vec3( uv, 0.0 );
}

void main( )
{
	mainImage( fragColor, vec2( gl_FragCoord.x, iResolution.y - gl_FragCoord.y ) );
}