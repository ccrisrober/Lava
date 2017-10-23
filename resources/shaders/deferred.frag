#version 450

layout(early_fragment_tests) in;

layout (location = 0 ) in vec2 uv;

layout (location = 0 ) out vec4 fragColor;

struct Light
{
    vec3 Position;
    vec3 Color;
};

const int NR_LIGHTS = 6;

layout( binding = 0 ) uniform UniformBufferObject
{
    Light lights[NR_LIGHTS];
    vec3 viewPos;
} ubo;

layout(binding = 1) uniform sampler2D gPosition;
layout(binding = 2) uniform sampler2D gNormal;
layout(binding = 3) uniform sampler2D gAlbedoSpec;

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, uv).rgb;
    vec3 Normal = texture(gNormal, uv).rgb;
    vec3 Albedo = texture(gAlbedoSpec, uv).rgb;
    float Specular = texture(gAlbedoSpec, uv).a;
    
    // then calculate lighting as usual
    vec3 lighting = Albedo * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(ubo.viewPos - FragPos);
    for( int i = 0; i < NR_LIGHTS; ++i )
    {
        // diffuse
        vec3 lightDir = normalize(ubo.lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * ubo.lights[i].Color;
        lighting += diffuse;
    }
    
    fragColor = vec4(lighting, 1.0);
}  