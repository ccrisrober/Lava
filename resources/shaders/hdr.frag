#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(binding = 0) uniform sampler2D tex;

layout(binding = 1) uniform UniformBufferObjectHDR
{
    bool enable_hdr;
    float exposure;
} uboHDR;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
	const float gamma = 2.2;
	vec3 result;
    vec3 hdrColor = texture(tex, uv).rgb;
    if(uboHDR.enable_hdr)
    {
        // reinhard
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        // exposure
        result = vec3(1.0) - exp(-hdrColor * uboHDR.exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
    }
    else
    {
        result = pow(hdrColor, vec3(1.0 / gamma));
    }
    fragColor = vec4(result, 1.0);
}