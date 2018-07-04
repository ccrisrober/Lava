#version 450

layout(binding = 0) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 projection;
  vec3 viewPos;
};

layout(push_constant) uniform PushConsts
{
  float particleSize;
};

layout ( points ) in;
layout ( triangle_strip, max_vertices = 4 ) out;

layout ( location = 0 ) in float vLevel[ ];

layout ( location = 0 ) out vec2 fUV;
layout ( location = 1 ) out float fLevel;

const float minPointScale = 0.1;
const float maxPointScale = 0.7;
const float maxDistance   = 100.0;

void main( )
{
  vec4 P = gl_in[0].gl_Position;

  float cameraDist = distance( P.xyz, viewPos );

  float pointSize = 1.0 - (cameraDist / maxDistance);
  pointSize = max( pointSize, minPointScale );
  pointSize = min( pointSize, maxPointScale );

  pointSize *= particleSize;

  // a: left-bottom 
  vec2 va = P.xy + vec2( -0.5, -0.5 ) * pointSize;
  gl_Position = projection * vec4( va, P.zw );
  fUV = vec2( 0.0, 0.0 );
  fLevel = vLevel[ 0 ];
  EmitVertex( );  
  
  // b: left-top
  vec2 vb = P.xy + vec2( -0.5, 0.5 ) * pointSize;
  gl_Position = projection * vec4( vb, P.zw );
  fUV = vec2( 0.0, 1.0 );
  fLevel = vLevel[ 0 ];
  EmitVertex( );  
  
  // d: right-bottom
  vec2 vd = P.xy + vec2( 0.5, -0.5 ) * pointSize;
  gl_Position = projection * vec4( vd, P.zw );
  fUV = vec2( 1.0, 0.0 );
  fLevel = vLevel[ 0 ];
  EmitVertex( );  

  // c: right-top
  vec2 vc = P.xy + vec2( 0.5, 0.5 ) * pointSize;
  gl_Position = projection * vec4( vc, P.zw );
  fUV = vec2( 1.0, 1.0 );
  fLevel = vLevel[ 0 ];
  EmitVertex( );  

  EndPrimitive( );  
}