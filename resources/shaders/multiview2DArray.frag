#version 450

layout( location = 0 ) in vec2 uv;
layout( location = 1 ) in float level;

layout(binding = 1) uniform sampler2DArray texSampler;

layout (location = 0) out vec4 fragColor;

void main( )
{
    fragColor = texture(texSampler, vec3(uv, level));
    if(fragColor.a < 1.0) discard;
}