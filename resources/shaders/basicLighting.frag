#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;

layout (location = 0) out vec4 outColor;

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
  
layout(binding = 1) uniform UniformBufferObject
{
    vec3 viewPos;
    Material material;
    Light light;
} ubo;


void main()
{
    // ambient
    vec3 ambient = ubo.light.ambient * ubo.material.ambient;
    
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(ubo.light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = ubo.light.diffuse * (diff * ubo.material.diffuse);
    
    // specular
    vec3 viewDir = normalize(ubo.viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ubo.material.shininess);
    vec3 specular = ubo.light.specular * (spec * ubo.material.specular);  
        
    vec3 result = ambient + diffuse + specular;
    outColor = vec4(result, 1.0);
} 