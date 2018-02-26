#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;

// constants
layout( push_constant ) uniform LightParameters
{
	vec4 Position;
} Light;

// outputs
layout(location = 0) out vec4 outColor;

// Blinn-Phong lighting model
void main() {
	vec3 normalVector = normalize( vertNormal );
	vec3 lightVector = normalize( Light.Position.xyz - vertPosition );
	
	// ambient
	float ambientTerm = 0.1;
	outColor = vec4( ambientTerm );
	
	// diffuse
	float diffuseTerm = max( 0.0, dot( normalVector, lightVector ) );
	outColor += vec4( diffuseTerm );
	
	// specular 
	if( diffuseTerm > 0.0 )
	{
		// In view space, camera position is (0.0, 0.0, 0.0)
		vec3 viewVector = normalize( vec3( 0.0 ) - vertPosition.xyz );
		vec3 halfVector = normalize( viewVector + lightVector );
		float shinniness = 60.0;
		float specularTerm = pow( dot( halfVector, normalVector ), shinniness );
		outColor += specularTerm;
	}
}