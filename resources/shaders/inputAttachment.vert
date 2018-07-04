#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform UBO 
{
  mat4 projection;
  mat4 view;
  mat4 model;
  vec3 lightPos;
};

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;

void main( ) 
{
  outNormal = inNormal;
  gl_Position = projection * view * model * vec4(inPos, 1.0);
  
  outNormal = mat3(transpose(inverse(model))) * inNormal;
  outPos = vec3(model * vec4(inPos, 1.0));
}