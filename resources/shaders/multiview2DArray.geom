#version 450

layout( invocations = 4, triangles ) in;

layout( triangle_strip, max_vertices = 12 ) out;

layout(binding = 0) uniform ubo0
{
    mat4 proj;
    mat4 view;
    mat4 model;
};

layout( location = 0 ) in vec2 inUV[ ];

layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out float level;

void main( )
{
  // Set the viewport index that the vertex will be emitted to
  gl_ViewportIndex = gl_InvocationID;
  for( int i = 0; i < gl_in.length( ); ++i )
  {
    outUV = inUV[ i ];
    level = float( gl_InvocationID );

    vec4 pos = gl_in[ i ].gl_Position;

    gl_Position = proj * view * model * pos;  // We can move this to vertex shader if only use a unique view and/or projection matrix.

    gl_PrimitiveID = gl_PrimitiveIDIn;
    EmitVertex( );
  }
  EndPrimitive( );
}