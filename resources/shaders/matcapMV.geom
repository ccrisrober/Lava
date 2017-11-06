#version 450

#extension GL_ARB_viewport_array : enable

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj[ 2 ];
} ubo;


layout(location = 0) in vec3 inNormal[];

layout(location = 0) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	for(int i = 0; i < gl_in.length(); ++i)
	{
	    gl_Position = ubo.proj[gl_InvocationID] * ubo.view * ubo.model * gl_in[i].gl_Position;
	    mat3 normalMatrix = mat3(inverse(transpose(ubo.view * ubo.model)));
	    outNormal = normalize(normalMatrix * inNormal[ i ]);
		// Set the viewport index that the vertex will be emitted to
		gl_ViewportIndex = gl_InvocationID;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		EmitVertex( );
	}
	EndPrimitive( );
}