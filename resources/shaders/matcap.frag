#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


//layout(binding = 1) uniform sampler2DArray texSampler;
layout(binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec3 outNormal;
layout (location = 0) out vec4 fragColor;

vec2 matcap(vec3 eye, vec3 normal)
{
    vec3 reflected = reflect(eye, normal);

    float m = 2.0 * sqrt(
        pow(reflected.x, 2.0) +
        pow(reflected.y, 2.0) +
        pow(reflected.z + 1.0, 2.0)
    );

    return reflected.xy / m + 0.5;
}

void main( )
{
	vec3 viewPos = -ubo.view[3].xyz * mat3(ubo.view);
    //fragColor = texture(texSampler, vec3(matcap(viewPos, normalize(outNormal)), 0.0));
    fragColor = texture(texSampler, matcap(viewPos, normalize(outNormal)));
}