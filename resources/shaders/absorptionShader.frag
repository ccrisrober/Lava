#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(set = 0, binding = 0) uniform sampler2D dataTexture;

layout(binding = 1) uniform UniformBufferObject
{
	vec3 Kd;
	float Sigma;
	bool TronEffect;
} ubo;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
	vec3 Color = vec3(1.0, 0.0, 0.0);
	vec2 tex = texture( dataTexture, uv ).rg;
	float thickness = abs( tex.r );
	if( thickness <= 0.0 ) discard;
	float sigma = 30.0;
    float fresnel = 1.0 - tex.g;
    float intensity = fresnel * exp(-sigma * thickness);
    fragColor = vec4(intensity * Color, 1.0);


    fragColor = vec4(thickness * 1.0, 0.0, 0.0, 1.0);

	//float fresnel = ubo.TronEffect ? tex.g : ( 1.0 - 0.5 * tex.g );
    //float intensity = fresnel * exp( -ubo.Sigma * thickness );    // http://omlc.org/classroom/ece532/class3/muadefinition.html
    //fragColor = vec4( intensity * ubo.Kd, 1.0 );

    //fragColor = vec4( vec3(tex.g), 1.0 );
}