#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 proj;
  vec3 viewPos;
};

layout (constant_id = 0) const int mode = 0;
layout (constant_id = 1) const float shininess = 0.0;

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;

layout (location = 0) out vec4 outColor;

const vec3 lightPos = vec3( 1.2, 1.0, 2.0 );

void main( void )
{
  vec3 ambientColor, diffuseColor, specularColor;

  if( mode == 1 )
  {
    // Emerald color
    ambientColor = vec3( 0.0215, 0.1745, 0.0215 );
    diffuseColor = vec3( 0.07568, 0.61424, 0.07568 );
    specularColor = vec3( 0.633, 0.727811, 0.633 );
  }
  else if ( mode == 2 )
  {
    // Gold color
    ambientColor = vec3( 0.24725, 0.1995, 0.0745 );
    diffuseColor = vec3( 0.75164, 0.60648, 0.22648 );
    specularColor = vec3( 0.628281, 0.555802, 0.366065 );
  }

  // ambient
  vec3 ambient = 0.2 * ambientColor;
  
  // diffuse 
  vec3 norm = normalize( Normal );
  vec3 lightDir = normalize( lightPos - FragPos );
  float diff = max( dot( norm, lightDir ), 0.0 );
  vec3 diffuse = 0.5 * (diff * diffuseColor);
  
  // specular
  vec3 viewDir = normalize( viewPos - FragPos );
  vec3 reflectDir = reflect(-lightDir, norm);  
  float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), shininess );
  vec3 specular = 1.0 * ( spec * specularColor );  
      
  vec3 result = ambient + diffuse + specular;
  outColor = vec4( result, 1.0 );
}


      

