#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
  float time;
} ubo;

layout (location = 0) out vec3 color;

vec4 explode(vec4 position, vec3 normal)
{
  float magnitude = 5.0f;
  if (gl_PrimitiveIDIn % 3 == 0)
  {
    vec3 direction = normal * ((sin(ubo.time) + 1.0) / 2.0) * magnitude;
    color = vec3( 1.0, 0.0, 0.0 );
    return position + vec4(direction, 0.0);
  }
  else if (gl_PrimitiveIDIn % 3 == 1)
  {
    vec3 direction = normal * ((cos(ubo.time) + 1.0) / 2.0) * magnitude;
    color = vec3( 0.0, 1.0, 0.0 );
    return position + vec4(direction, 0.0);
  }
  else
  {
    vec3 direction = normal * ((tan(ubo.time) + 1.0) / 2.0) * magnitude;
    color = vec3( 0.0, 0.0, 1.0 );
    return position;// + vec4(direction, 0.0);
  }
}
vec3 GetNormal()
{
  vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
  vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
  return normalize(cross(a, b));
}

void main( void )
{
  vec3 normal = GetNormal();
  gl_Position = explode(gl_in[0].gl_Position, normal);
  EmitVertex();
  gl_Position = explode(gl_in[1].gl_Position, normal);
  EmitVertex();
  gl_Position = explode(gl_in[2].gl_Position, normal);
  EmitVertex();
  EndPrimitive();
}