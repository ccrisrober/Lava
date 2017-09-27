#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float amount;
} ubo;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat3 normalMatrix = mat3(inverse(transpose(ubo.model)));
    vec3 pos = inPosition;
    outUV = inUV;
    const vec2 size = vec2(0.5, 0.0);
    const ivec3 off = ivec3(-1, 0, 1);
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
    vec4 pp = ubo.model * vec4(pos, 1.0);
    pp = ubo.view * pp;
    gl_Position = ubo.proj * pp;
}