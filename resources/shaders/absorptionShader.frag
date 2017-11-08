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

const vec3 DiffuseMaterial = vec3(0.0, 1.0, 0.0);

void main( void )
{
	vec2 tex = texture( dataTexture, uv ).rg;
	float thickness = abs( tex.r );
	if( thickness <= 0.0 ) discard;
    float fresnel = ubo.TronEffect ? tex.g : ( 1.0 - 0.5 * tex.g );
    float intensity = fresnel * exp( -ubo.Sigma * thickness );    // http://omlc.org/classroom/ece532/class3/muadefinition.html
    fragColor = vec4( intensity * ubo.Kd, 1.0 );
}