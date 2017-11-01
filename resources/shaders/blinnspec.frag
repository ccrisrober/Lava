#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

layout(binding = 1) uniform sampler2D texAlbedo;
layout(binding = 2) uniform sampler2D texSpecular;

layout (location = 0) out vec4 FragColor;

layout(std140, binding = 3) uniform UniformBufferObject
{
    //bool mode;
    vec4 lightPositions[ 4 ];
    vec4 lightColors[ 4 ];
    vec3 viewPos;
} ubo;

layout(push_constant) uniform PushConsts
{
    bool gamma;
} pushConsts;


vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(ubo.viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);

    vec3 specColor = texture( texSpecular, TexCoord ).rgb;

    vec3 specular = spec * lightColor * specColor;

    // simple attenuation
    float max_distance = 1.5;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (pushConsts.gamma ? distance * distance : distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return diffuse + specular;
}

void main()
{
    vec3 color = texture(texAlbedo, TexCoord).rgb;
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        lighting += BlinnPhong(normalize(Normal), FragPos, ubo.lightPositions[i].xyz, ubo.lightColors[ i ].rgb);
    }

    color *= lighting;
    if(pushConsts.gamma)
    {
        color = pow(color, vec3(1.0/2.2));
    }
    FragColor = vec4(color, 1.0);
}