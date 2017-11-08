#version 450

#extension GL_ARB_viewport_array : enable

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view[ 2 ];
    mat4 proj[ 2 ];
} ubo;

layout(location = 0) in vec3 inNormal[];

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 viewPos;

void main()
{
	for(int i = 0; i < gl_in.length(); ++i)
	{
		vec4 pos = gl_in[i].gl_Position;
	    gl_Position = ubo.proj[gl_InvocationID] * 
	    	ubo.view[gl_InvocationID] * ubo.model * 
	    	pos;
    	
	    mat3 normalMatrix = mat3(inverse(transpose(
	    	ubo.view[gl_InvocationID] * ubo.model)));

	    outNormal = normalize(normalMatrix * inNormal[ i ]);
	    
		
		viewPos = -ubo.view[gl_InvocationID][3].xyz * mat3(ubo.view[gl_InvocationID]);

		// Set the viewport index that the vertex will be emitted to
		gl_ViewportIndex = gl_InvocationID;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		EmitVertex( );
	}
	EndPrimitive( );
}