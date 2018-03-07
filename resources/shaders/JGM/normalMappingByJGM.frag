#version 450
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout( location = 0 ) in vec3 vertPosition;
layout( location = 1 ) in vec3 vertNormal;
layout( location = 2 ) in vec2 vertTexCoord;
layout( location = 3 ) in vec3 vertTangent;
layout( location = 4 ) in vec3 vertBitangent;  

// descriptor sets
layout( set = 0, binding = 1 ) uniform sampler2D ogreDiffuseMapSampler; 
layout( set = 0, binding = 2 ) uniform sampler2D ogreNormalMapSampler; 

// push constants
layout( push_constant ) uniform NormalMappingPushConstants
{
  layout( offset = 64 ) vec4 lightPosition;
} nmpc;

// outputs
layout( location = 0 ) out vec4 outColor;
	
// auxiliar
mat3 btn = mat3( vertTangent, vertBitangent, vertNormal );	
	
void main()	{
  vec3 normal = 2 * texture( ogreNormalMapSampler, vertTexCoord ).rgb - 1.0;
  vec3 normalVector = normalize( btn * normal );
  vec3 lightVector = normalize( nmpc.lightPosition.xyz - vertPosition );
  vec4 textureColor = texture( ogreDiffuseMapSampler, vertTexCoord );
  float ambientIntensity = 0.1;
  outColor += ambientIntensity * textureColor;
  float diffuseIntensity = max( 0.0, dot( normalVector, lightVector ) ) * max( 0.0, dot( vertNormal, lightVector ) );
  outColor += diffuseIntensity * textureColor;
  if( diffuseIntensity > 0.0 )
  {
    vec4 specularColor = vec4( 0.83, 0.36, 0.68, 1.0 );
    vec3 halfVector = normalize( normalize( -vertPosition.xyz ) + lightVector );
    float specularIntensity = pow( dot( halfVector, normalVector ), 60.0 );
    outColor += specularIntensity * specularColor;
  }
}