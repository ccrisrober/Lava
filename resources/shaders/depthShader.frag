#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;

layout (location = 0) out vec2 fragColor;

const float ExpFresnel = 3.0;

void main( )
{
    vec3 N = normalize(Normal);
    vec3 P = FragPos;
    vec3 I = normalize( P );
    float cosTheta = abs( dot( I, N ) );
    float fresnel = pow( 1.0 - cosTheta, ExpFresnel );
    float depth = gl_FrontFacing ? gl_FragCoord.z : -gl_FragCoord.z;
    fragColor = vec2(depth, fresnel);
} 