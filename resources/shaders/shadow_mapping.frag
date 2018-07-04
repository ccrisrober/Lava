#version 450

layout( location = 0 ) in vec3 FragPos;
layout( location = 1 ) in vec3 Normal;
//layout( location = 2 ) in vec2 TexCoords;
layout( location = 2 ) in vec4 FragPosLightSpace;

layout( location = 0 ) out vec4 fragColor;

layout(binding = 0) uniform ubo0
{
  mat4 projection;
  mat4 view;
  mat4 model;
  vec3 lightPos;
  vec3 viewPos;
};

//layout( binding = 2 ) uniform sampler2D diffuseTexture;
layout( binding = 2 ) uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // calculate bias (based on depth map resolution and slope)
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  // check whether current frag pos is in shadow
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    }    
  }
  shadow /= 9.0;
  
  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if(projCoords.z > 1.0)
  {
    shadow = 0.0;
  }
    
  return shadow;
}

void main()
{           
  vec3 color = vec3( 239.0 / 255.0, 243.0 / 255.0, 208.0 / 255.0 ); //texture(diffuseTexture, TexCoords).rgb;
  vec3 normal = normalize(Normal);
  vec3 lightColor = vec3( 253.0 / 255.0, 198.0 / 255.0, 15.0 / 255.0 );
  // ambient
  vec3 ambient = 0.3 * color;
  // diffuse
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;
  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
  vec3 specular = spec * lightColor;    
  // calculate shadow
  float shadow = ShadowCalculation(FragPosLightSpace);                      
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color; 

  fragColor = vec4(lighting, 1.0);
}