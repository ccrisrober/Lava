#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 aPos;
layout(location = 0) out vec3 uv; //3D texture coordinates for texture lookup in the fragment shader

out gl_PerVertex
{
    vec4 gl_Position;
};

void main( )
{
	//get the clipspace position 
    gl_Position = ubo.proj * ubo.model * ubo.view * vec4(aPos, 1.0);
    
	//get the 3D texture coordinates by adding (0.5,0.5,0.5) to the object space 
	//vertex position. Since the unit cube is at origin (min: (-0.5,-0.5,-0.5) and max: (0.5,0.5,0.5))
	//adding (0.5,0.5,0.5) to the unit cube object space position gives us values from (0,0,0) to 
	//(1,1,1)
    uv = aPos + vec3( 0.5 );
}