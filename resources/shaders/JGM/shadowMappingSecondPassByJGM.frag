#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout( location = 0 ) in vec3 vertNormal;
layout( location = 1 ) in vec4 vertTexCoords;
layout( location = 2 ) in vec3 vertLight;

// uniforms
layout( set = 0, binding = 1 ) uniform sampler2D ShadowMap; 

// outputs
layout( location = 0 ) out vec4 outColor;

void main() {
	float shadow = 1.0;
	vec4 shadowCoords = vertTexCoords / vertTexCoords.w;
	
	if( texture( ShadowMap, shadowCoords.xy ).r < shadowCoords.z -0.005 )
	{
	  shadow = 0.5;
	}
	
	vec3 normalVector = normalize( vertNormal );
	vec3 lightVector = normalize( vertLight );
	
	// ambient
	float ambientTerm = 0.2;
	outColor = vec4( ambientTerm );
	
	// diffuse
	float diffuseTerm = max( 0.0, dot( normalVector, lightVector ) );
	outColor += shadow * vec4( diffuseTerm );
}