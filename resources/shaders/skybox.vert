#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
} cameraUBO ;

layout (location = 0) out vec3 outUVW;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main( void ) 
{
    outUVW = inPos;

    // Remove the translation from camera view matrix
    mat4 cameraView = cameraUBO.view;
    cameraView[ 3 ][ 0 ] = 0.0;
    cameraView[ 3 ][ 1 ] = 0.0;
    cameraView[ 3 ][ 2 ] = 0.0;

    gl_Position = cameraUBO.projection * cameraView * vec4( inPos, 1.0 );
    gl_Position.y = -gl_Position.y;
}
