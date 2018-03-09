#version 450
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout( location = 0 ) in vec3 vertPosition;
layout( location = 1 ) in vec3 vertNormal;  

// descriptor sets
layout( set = 0, binding = 1 ) uniform samplerCube cubeMapSampler; 

// push constants
layout( push_constant ) uniform CubeMapRRPushConstants
{
  layout( offset = 64 ) vec4 cameraPosition;
} cmrrpc;

// outputs
layout( location = 0 ) out vec4 outColor;
		
void main()	{
  vec3 viewVector = vertPosition - cmrrpc.cameraPosition.xyz;
  
  float angle = smoothstep( 0.3, 0.7, dot( normalize( -viewVector ), vertNormal ) );

  vec3 reflectVector = reflect( viewVector, vertNormal );
  vec4 reflectColor = texture( cubeMapSampler, reflectVector );
  
  vec3 refractVector = refract( viewVector, vertNormal, 0.3 );
  vec4 refractColor = texture( cubeMapSampler, refractVector );
  
  outColor = mix( reflectColor, refractColor, angle );
}