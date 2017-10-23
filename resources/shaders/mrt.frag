#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout( location = 0 ) in vec3 FragPos;
layout( location = 1 ) in vec3 Normal;
layout( location = 2 ) in vec2 TexCoords;

layout(binding = 1) uniform sampler2D textDiffuse;
layout(binding = 2) uniform sampler2D textSpecular;

layout( location = 0 ) out vec3 gPosition;
layout( location = 1 ) out vec3 gNormal;
layout( location = 2 ) out vec4 gAlbedoSpec;

void main( )
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(textDiffuse, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(textSpecular, TexCoords).r;
}