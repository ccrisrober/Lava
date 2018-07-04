#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D noiseTex;
layout(binding = 2) uniform ubo2
{
	float scale;
	float intensity;
    vec3 viewPos;
};
layout(binding = 3) uniform sampler2D diffuseTex;

layout( location = 0 ) in vec3 Position;
layout( location = 1 ) in vec3 Normal;
layout( location = 2 ) in vec2 UV;

layout( location = 0 ) out vec4 outColor;

float saturate( float v )
{
	return clamp( v, 0.0, 1.0 );
}

void main( )
{
	vec3 sparkleMap = texture( noiseTex, scale * UV ).rgb;
	sparkleMap -= vec3( 0.5, 0.5, 0.5 );
	sparkleMap = normalize( normalize(sparkleMap) * Normal);

	vec3 viewDirection = normalize( Position - viewPos );
	float sparkle = dot( -viewDirection, sparkleMap );
	sparkle = pow( saturate( sparkle ), intensity );
	vec3 sparkleColor = vec3( sparkle );

	vec3 lightColor = vec3( 1.0 );
	vec3 lightPos = vec3( 2.5 );

    // ambient
    float ambientStrength = 0.01;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - Position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;


	vec3 objectColor = vec3( texture( diffuseTex, UV ) );

    vec3 result = (ambient + diffuse + sparkleColor) * objectColor;

	outColor = vec4( result, 1.0 );
}