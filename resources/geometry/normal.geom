#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (location = 0) in vec3 inNormal[];

layout( location = 0 ) out vec3 color;

const float MAGNITUDE = 0.25;

void GenerateLine(int index)
{
  gl_Position = gl_in[index].gl_Position;
  EmitVertex();
  gl_Position = gl_in[index].gl_Position + vec4(inNormal[index], 0.0) * MAGNITUDE;
  color = inNormal[index];
  EmitVertex( );
  EndPrimitive( );
}

void main( void )
{
  GenerateLine( 0 ); // first vertex normal
  GenerateLine( 1 ); // second vertex normal
  GenerateLine( 2 ); // third vertex normal
}