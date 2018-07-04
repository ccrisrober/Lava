#version 450

layout( set = 0, binding = 1 ) uniform sampler2D sColorMap;
layout( set = 1, binding = 1 ) uniform sampler2D ProjectorTex;

layout( location = 0 ) in vec3 EyeNormal; // Normal in eye coordinates
layout( location = 1 ) in vec4 EyePosition; // Position in eye coordinates
layout( location = 2 ) in vec4 ProjTexCoord;

layout( location = 0 ) out vec4 fragColor;

void main() {
	vec3 color = vec3( 1.0 ); //phongModel(vec3(EyePosition), EyeNormal);
	vec4 projTexColor = vec4(0.0);
	if( ProjTexCoord.z > 0.0 )
	{
		projTexColor = textureProj(ProjectorTex, ProjTexCoord);
	}
	fragColor = vec4(color, 1.0) + projTexColor * 0.5;
}