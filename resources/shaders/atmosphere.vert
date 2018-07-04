#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
    mat4 mvp;
};

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec2 texCoord;

layout( location = 1 ) out vec2 TexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
    gl_Position = mvp * vec4(position, 1.0);
    TexCoord = texCoord;
}