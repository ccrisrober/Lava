#version 450

#extension GL_ARB_viewport_array : enable

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform UBO 
{
	mat4 projection[2];
	mat4 view[2];
	mat4 model;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outNormal;

void main( void )
{	
	for( int i = 0; i < gl_in.length(); ++i )
	{
    	mat3 normal = mat3(transpose(inverse(ubo.view[ gl_InvocationID ] * ubo.model)));
		outNormal = normal * inNormal[i];

		vec4 pos = gl_in[i].gl_Position;
		vec4 worldPos = (ubo.view[ gl_InvocationID ] * ubo.model * pos);
		
		gl_Position = ubo.projection[ gl_InvocationID ] * worldPos;

		// Set the viewport index that the vertex will be emitted to
		gl_ViewportIndex = gl_InvocationID;
      	gl_PrimitiveID = gl_PrimitiveIDIn;
		EmitVertex( );
	}
	EndPrimitive( );
}