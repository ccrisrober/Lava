#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( points ) in;
layout ( line_strip, max_vertices = 64 ) out;

layout ( location = 0 ) in vec3 vColor[ ];
layout ( location = 1 ) in uint vSides[ ];

layout ( location = 0 ) out vec3 fColor;

const float PI = 3.1415926;

void main( )
{
  fColor = vColor[ 0 ];

  int sides = int( vSides[ 0 ] );

  for ( int i = 0; i <= sides; ++i )
  {
    // Angle between each side in radians
    float ang = PI * 2.0 / float( vSides[ 0 ] ) * i;

    // Offset from center of point (0.2 to accomodate for aspect ratio)
    vec4 offset = vec4( cos( ang ) * 0.2, -sin( ang ) * 0.4, 0.0, 0.0 );
    gl_Position = gl_in[ 0 ].gl_Position + offset;

    EmitVertex( );
  }

  EndPrimitive( );
}