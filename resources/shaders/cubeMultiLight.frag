#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoords;

layout(binding = 1) uniform sampler2D texDiffuse;
layout(binding = 2) uniform sampler2D texSpecular;

struct DirLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
    vec3 position;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};
const float constant = 1.0;
const float linear = 0.14;
const float quadratic = 0.07;

#define NR_POINT_LIGHTS 4

layout(binding = 3) uniform UniformBufferObject
{
	vec3 viewPos;
	DirLight dirLight;
	PointLight pointLights[NR_POINT_LIGHTS];
	SpotLight spotLight;
} ubo;

layout (location = 0) out vec4 outColor;

const float shininess = 32.0;

// calculates the color when using a directional light.
vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-ubo.dirLight.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = ubo.dirLight.ambient * vec3(texture(texDiffuse, TexCoords));
    vec3 diffuse = ubo.dirLight.diffuse * diff * vec3(texture(texDiffuse, TexCoords));
    vec3 specular = ubo.dirLight.specular * spec * vec3(texture(texSpecular, TexCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(vec3 light_position, vec3 light_ambient, vec3 light_diffuse, 
    vec3 light_specular, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light_position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(light_position - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light_ambient * vec3(texture(texDiffuse, TexCoords));
    vec3 diffuse = light_diffuse * diff * vec3(texture(texDiffuse, TexCoords));
    vec3 specular = light_specular * spec * vec3(texture(texSpecular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(ubo.spotLight.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(ubo.spotLight.position - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-ubo.spotLight.direction)); 
    float epsilon = ubo.spotLight.cutOff - ubo.spotLight.outerCutOff;
    float intensity = clamp((theta - ubo.spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = ubo.spotLight.ambient * vec3(texture(texDiffuse, TexCoords));
    vec3 diffuse = ubo.spotLight.diffuse * diff * vec3(texture(texDiffuse, TexCoords));
    vec3 specular = ubo.spotLight.specular * spec * vec3(texture(texSpecular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
void main( )
{
  	// properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(ubo.viewPos - FragPos);
    
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result = vec3(0.0);

    result += CalcDirLight(norm, viewDir);
    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        result += CalcPointLight(ubo.pointLights[i].position, 
            ubo.pointLights[i].ambient, ubo.pointLights[i].diffuse, ubo.pointLights[i].specular,
            norm, FragPos, viewDir
        );    
    }
    // phase 3: spot light
    result += CalcSpotLight(norm, FragPos, viewDir);

    //result = vec3(texture(texDiffuse, TexCoords));

    //result += Normal;
    
    outColor = vec4(result, 1.0);
}