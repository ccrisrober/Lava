#version 450

layout (location = 0) in vec3 outPos;
layout (location = 1) in vec3 outNormal;

layout (binding = 0) uniform UBO 
{
  mat4 projection;
  mat4 view;
  mat4 model;
  vec3 lightPos;
};

layout (location = 0) out vec4 fragColor;

const vec3 lightColor = vec3(0.0, 1.0, 0.0);
const vec3 material_ambient = vec3(1.0, 0.5, 0.31);
const vec3 material_diffuse = vec3(1.0, 0.5, 0.31);
const vec3 material_specular = vec3(0.5, 0.5, 0.5);
const float material_shininess = 32.0;

void main( void ) 
{ 
  vec3 viewPos = -view[3].xyz * mat3(view);
  // ambient
  vec3 ambient = lightColor * material_ambient;
  
  // diffuse 
  vec3 norm = normalize(outNormal);
  vec3 lightDir = normalize(lightPos - outPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = lightColor * (diff * material_diffuse);
  
  // specular
  vec3 viewDir = normalize(viewPos - outPos);
  vec3 reflectDir = reflect(-lightDir, norm);  
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
  vec3 specular = lightColor * (spec * material_specular);  
      
  vec3 result = ambient + diffuse + specular;

  float intensity = dot(norm, lightDir);

  float shade = 1.0;
  shade = intensity < 0.5 ? 0.75 : shade;
  shade = intensity < 0.35 ? 0.6 : shade;
  shade = intensity < 0.25 ? 0.5 : shade;
  shade = intensity < 0.1 ? 0.25 : shade;

  fragColor = vec4( result * 3.0 * shade, 1.0 );
}