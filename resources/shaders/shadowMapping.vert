#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( binding = 0 ) uniform UBO
{
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
};

layout( push_constant ) uniform PushConstant
{
	mat4 model;
};

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec2 inTexCoords;

layout( location = 0 ) out vec3 FragPos;
layout( location = 1 ) out vec3 Normal;
layout( location = 2 ) out vec2 TexCoords;
layout( location = 3 ) out vec4 FragPosLightSpace;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    FragPos = vec3(model * vec4(inPosition, 1.0));
    Normal = transpose(inverse(mat3(model))) * inNormal;
    TexCoords = inTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * model * vec4(inPosition, 1.0);
}