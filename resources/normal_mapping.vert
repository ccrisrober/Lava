#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 lightPos;
	vec4 cameraPos;
} ubo;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec2 TexCoord;

void main(void) 
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0f);
    TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
    mat3 normalMatrix = mat3(transpose(inverse( ubo.model )));
    Normal = normalMatrix * normal;
    outPosition = vec3( ubo.model * vec4( position, 1.0 ) );
}
