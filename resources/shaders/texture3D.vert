#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (binding = 0) uniform UBO 
{
	float depth;
} ubo;

layout(location = 0) out vec3 uv;

vec2 positions[4] = vec2[]
(
	vec2(-1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0)
);

void main( void )
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	uv = vec3( vec2( positions[gl_VertexIndex] * 0.5 + 0.5 ), ubo.depth);
}