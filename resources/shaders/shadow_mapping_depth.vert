#version 450

layout(binding = 0) uniform ubo0
{
  mat4 projection;
  mat4 view;
  mat4 model;
  vec3 lightPos;
  vec3 viewPos;
};

layout(binding = 1) uniform ubo1
{
    mat4 lightSpaceMatrix;
};

layout( location = 0 ) in vec3 position;

void main( )
{
    gl_Position = lightSpaceMatrix * model * vec4( position, 1.0 );
}