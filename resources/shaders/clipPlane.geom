#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(binding = 0) uniform UniformBufferObject
{
  mat4 model;
  mat4 view;
  mat4 proj;
  vec4 ClipPlane;
};

layout(location = 0) in vec3 vPosition[3];
layout (location = 0) out vec2 gDistance;
layout (location = 1) out vec3 gNormal;

void main( )
{
  vec3 A = vPosition[0];
  vec3 B = vPosition[1];
  vec3 C = vPosition[2];

  mat3 NormalMatrix = mat3(inverse(transpose(view * model)));
  
  gNormal = NormalMatrix * normalize(cross(B - A, C - A));
  
  gDistance = vec2(1, 0);
  gl_Position = gl_in[0].gl_Position;
  gl_ClipDistance[0] = gl_in[0].gl_ClipDistance[0];
  EmitVertex();

  gDistance = vec2(0, 0);
  gl_Position = gl_in[1].gl_Position;
  gl_ClipDistance[0] = gl_in[1].gl_ClipDistance[0];
  EmitVertex();

  gDistance = vec2(0, 1);
  gl_Position = gl_in[2].gl_Position;
  gl_ClipDistance[0] = gl_in[2].gl_ClipDistance[0];
  EmitVertex();

  EndPrimitive();
}