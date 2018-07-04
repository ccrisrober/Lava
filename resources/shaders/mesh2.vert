#version 450

layout(binding = 0) uniform ubo0
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 normal;

layout( location = 0 ) out vec3 outPosition;
layout( location = 1 ) out flat vec3 Normal;

void main( )
{
    gl_Position = proj * view * model * vec4(position, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse( model )));
    Normal = normalMatrix * normal;
    outPosition = vec3( model * vec4( position, 1.0 ) );
}