#version 450

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
	vec3 lightColor;
	vec3 cameraPos;
};

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) in vec2 texCoord;

layout( location = 0 ) out vec3 outPosition;
layout( location = 1 ) out vec3 Normal;
layout( location = 2 ) out vec2 TexCoord;

void main( )
{
    gl_Position = proj * view * model * vec4(position, 1.0);
    TexCoord = texCoord;
    mat3 normalMatrix = mat3(transpose(inverse( model )));
    Normal = normalMatrix * normal;
    outPosition = vec3( model * vec4( position, 1.0 ) );
}