#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 outPos;
layout (location = 1) in vec3 outNormal;
//layout (location = 2) in vec2 outUV;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec3 lightPos;
} ubo;


layout (location = 0) out vec4 fragColor;

const vec3 lightColor = vec3(0.0, 1.0, 0.0);
const vec3 material_ambient = vec3(1.0, 0.5, 0.31);
const vec3 material_diffuse = vec3(1.0, 0.5, 0.31);
const vec3 material_specular = vec3(0.5, 0.5, 0.5);
const float material_shininess = 32.0;

void main( void ) 
{	
	vec3 viewPos = -ubo.view[3].xyz * mat3(ubo.view);
    // ambient
    vec3 ambient = lightColor * material_ambient;
  	
    // diffuse 
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(ubo.lightPos - outPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material_diffuse);
    
    // specular
    vec3 viewDir = normalize(viewPos - outPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = lightColor * (spec * material_specular);  
        
    vec3 result = ambient + diffuse + specular;

	fragColor = vec4( result, 1.0 );
}