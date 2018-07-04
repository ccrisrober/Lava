#version 430
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform samplerBuffer texels;
layout( location = 0 ) out vec3 outColor;

vec2 vertices[ 3 ] = vec2[ ](
    vec2(  0.0, -0.5 ),
    vec2(  0.5,  0.5 ),
    vec2( -0.5,  0.5 )
);

void main( )
{
    outColor = texelFetch( texels, gl_VertexIndex % 3 ).rgb;

    gl_Position = vec4( vertices[ gl_VertexIndex % 3 ], 0.0, 1.0 );
}