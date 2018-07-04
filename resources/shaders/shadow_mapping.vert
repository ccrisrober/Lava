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

layout( location = 0 ) in vec3 aPos;
layout( location = 1 ) in vec3 aNormal;
//layout( location = 2 ) in vec2 aTexCoords;

layout( location = 0 ) out vec3 FragPos;
layout( location = 1 ) out vec3 Normal;
//layout( location = 2 ) out vec2 TexCoords;
layout( location = 2 ) out vec4 FragPosLightSpace;

void main( )
{
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = transpose(inverse(mat3(model))) * aNormal;
  //TexCoords = aTexCoords;
  FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}