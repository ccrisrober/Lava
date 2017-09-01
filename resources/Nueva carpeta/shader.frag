#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout( push_constant ) uniform ColorBlock
{
	vec4 Light1;
	vec4 Light2;
	vec4 Light3;
} PushConstant;

layout(location = 0) in vec2 fragTexCoord;
//layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
    //outColor.rgb  = PushConstant.Light1.rgb;
    outColor.rgb += PushConstant.Light2.rgb;
    //outColor.rgb *= PushConstant.Light3.rgb;
}