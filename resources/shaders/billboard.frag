#version 450
layout( location = 0 ) in vec2 uv;
layout( location = 0 ) out vec4 fragColor;

void main( )
{
	float alpha = 1.0 - dot( uv, uv );
	if ( 0.2 > alpha )
	{
		fragColor = vec4( alpha );
	}
}