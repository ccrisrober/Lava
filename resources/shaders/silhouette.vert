#version 450

layout(binding = 0) uniform UniformBufferObject
{
  mat4 model;
  mat4 view;
  mat4 proj;
};

layout( location = 0 ) in vec3 VertexPosition;
layout( location = 1 ) in vec3 VertexNormal;

layout( location = 0 ) out vec3 VPosition;
layout( location = 1 ) out vec3 VNormal;

void main( )
{
  mat4 ModelViewMatrix = view * model;
  mat3 NormalMatrix = mat3( transpose( inverse( ModelViewMatrix ) ) );
  
  gl_Position = proj * ModelViewMatrix * vec4( VertexPosition, 1.0 );
  VPosition = vec3( view * model * vec4( VertexPosition, 1.0 ) );
  VNormal = normalize( NormalMatrix * VertexNormal);
}