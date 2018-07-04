#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 proj;
};

layout( location = 0 ) in vec3 outPosition;
layout( location = 1 ) in flat vec3 Normal;

layout( location = 0 ) out vec4 fragColor;

void main( void )
{
  vec3 viewPos = -view[3].xyz * mat3(view);
  vec3 lightColor = vec3( 0.0, 1.0, 0.0 );
  vec3 meshColor = vec3( 0.0, 0.0, 1.0 );
  vec3 lightPos = vec3( 0.0, 2.0, 2.0 );

  vec3 ambient = vec3(0.4);

  vec3 norm = normalize(Normal);

  vec3 lightDir = normalize(lightPos - outPosition);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = lightColor * diff;

  vec3 viewDir = normalize(viewPos - outPosition);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = vec3(spec);

  fragColor = vec4((ambient + diffuse + specular) * meshColor, 1.0);
}