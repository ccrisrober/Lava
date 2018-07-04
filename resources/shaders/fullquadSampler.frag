#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout (set = 0, binding = 0) uniform texture2D tex;
layout (set = 0, binding = 1) uniform sampler samp;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
	vec2 uv_ = uv * 5.0;
	fragColor = texture(sampler2D(tex, samp), uv_);
}