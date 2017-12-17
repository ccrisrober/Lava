#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
    mat4 lightSpaceMatrix;
};

layout(push_constant) uniform PushConstant
{
	mat4 model;
};

layout(location = 0) in vec3 inPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    gl_Position = lightSpaceMatrix * model * vec4( inPosition, 1.0 );
}