#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// PN patch data
struct PnPatch
{
	float b210;
	float b120;
	float b021;
	float b012;
	float b102;
	float b201;
	float b111;
	float n110;
	float n011;
	float n101;
};

layout(binding = 0) uniform UniformBufferObject
{
	float tessLevel;
} ubo;

layout( vertices = 3 ) out;

layout( location = 0 ) in vec3 inNormal[ ];

layout( location = 0)  out vec3 outNormal[ 3 ];
layout( location = 3 ) out PnPatch outPatch[ 3 ];

float wij(int i, int j)
{
	return dot(gl_in[j].gl_Position.xyz - 
		gl_in[i].gl_Position.xyz, inNormal[i]);
}

float vij(int i, int j)
{
	vec3 Pj_minus_Pi = gl_in[j].gl_Position.xyz
					- gl_in[i].gl_Position.xyz;
	vec3 Ni_plus_Nj  = inNormal[i]+inNormal[j];
	return 2.0*dot(Pj_minus_Pi, Ni_plus_Nj)/dot(Pj_minus_Pi, Pj_minus_Pi);
}

#define ID gl_InvocationID


void main( )
{
	// get data
	gl_out[ID].gl_Position = gl_in[ID].gl_Position;
	outNormal[ID] = inNormal[ID];

	// set base 
	float P0 = gl_in[0].gl_Position[ID];
	float P1 = gl_in[1].gl_Position[ID];
	float P2 = gl_in[2].gl_Position[ID];
	float N0 = inNormal[0][ID];
	float N1 = inNormal[1][ID];
	float N2 = inNormal[2][ID];

	// compute control points
	outPatch[ID].b210 = (2.0 * P0 + P1 - wij(0, 1) * N0) / 3.0;
	outPatch[ID].b120 = (2.0 * P1 + P0 - wij(1, 0) * N1) / 3.0;
	outPatch[ID].b021 = (2.0 * P1 + P2 - wij(1, 2) * N1) / 3.0;
	outPatch[ID].b012 = (2.0 * P2 + P1 - wij(2, 1) * N2) / 3.0;
	outPatch[ID].b102 = (2.0 * P2 + P0 - wij(2, 0) * N2) / 3.0;
	outPatch[ID].b201 = (2.0 * P0 + P2 - wij(0, 2) * N0) / 3.0;
	float E = ( outPatch[ID].b210
			+ outPatch[ID].b120
			+ outPatch[ID].b021
			+ outPatch[ID].b012
			+ outPatch[ID].b102
			+ outPatch[ID].b201 ) / 6.0;
	float V = (P0 + P1 + P2) / 3.0;
	outPatch[ID].b111 = E + (E - V) * 0.5;
	outPatch[ID].n110 = N0 + N1 - vij(0, 1) * (P1 - P0);
	outPatch[ID].n011 = N1 + N2 - vij(1, 2) * (P2 - P1);
	outPatch[ID].n101 = N2 + N0 - vij(2, 0) * (P0 - P2);

	// set tess levels
	gl_TessLevelOuter[ID] = ubo.tessLevel;
	gl_TessLevelInner[0] = ubo.tessLevel;
}