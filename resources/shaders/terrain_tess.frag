#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( binding = 2 ) uniform sampler2D texSampler;

layout( location = 0 ) in vec2 outUV;
layout( location = 0 ) out vec4 fragColor;

void main( )
{
    fragColor = texture(texSampler, outUV);
    //fragColor.rgb = vec3( 1.0 );
}