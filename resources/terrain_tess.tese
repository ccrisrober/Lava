#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(triangles, equal_spacing, ccw) in;

layout(location = 0) in vec2 uv[ ];
layout(location = 0) out vec2 outUV;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    float amount;
    float tess_level;
} ubo;
layout(binding = 1) uniform sampler2D texSampler;

void main( void )
{
    vec4 pos = (gl_TessCoord.x * gl_in[ 0 ].gl_Position) +
        (gl_TessCoord.y * gl_in[ 1 ].gl_Position) +
        (gl_TessCoord.z * gl_in[ 2 ].gl_Position);

    outUV = (gl_TessCoord.x * uv[ 0 ]) +
        (gl_TessCoord.y * uv[ 1 ]) +
        (gl_TessCoord.z * uv[ 2 ]);

    const vec2 size = vec2(0.25, 0.0);
    const ivec3 off = ivec3(-0.15, 0.15, 0.15);
    vec4 wave = texture(texSampler, outUV);
    float s11 = wave.x;
    float s01 = textureOffset(texSampler, outUV, off.xy).x;
    float s21 = textureOffset(texSampler, outUV, off.zy).x;
    float s10 = textureOffset(texSampler, outUV, off.yx).x;
    float s12 = textureOffset(texSampler, outUV, off.yz).x;
    vec3 va = normalize(vec3(size.xy, s21 - s01));
    vec3 vb = normalize(vec3(size.yx, s12 - s10));
    vec4 bump = vec4(cross(va, vb), s11);
    pos.z += ubo.amount * bump.w;

    gl_Position = ubo.model * ubo.view * ubo.model * pos;
}