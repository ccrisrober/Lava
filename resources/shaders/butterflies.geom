#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (points) in;
layout (triangle_strip, max_vertices = 8) out;

#define PI 3.1415
vec2 size = vec2(1.0, 2.0);

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
    float up;
    float beta;
} ubo;

layout(location = 0) out vec2 TexCoord;

void generateQuad( in mat4 Transf, int sign )
{
    vec4 Position = gl_in[0].gl_Position;
    gl_Position = Position;
    gl_Position.xy += vec2(sign * size.x, -size.y / 2.0);
    gl_Position = Transf * gl_Position;
    TexCoord = vec2(0.0, 0.0);
    EmitVertex();
    gl_Position = Position;
    gl_Position.xy += vec2(sign * size.x, size.y / 2.0);
    gl_Position = Transf * gl_Position;
    TexCoord = vec2(0.0, 1.0);
    EmitVertex();
    gl_Position = Position;
    gl_Position.xy += vec2(0.0, -size.y / 2.0);
    gl_Position = Transf * gl_Position;
    TexCoord = vec2(1.0, 0.0);
    EmitVertex();
    gl_Position = Position;
    gl_Position.xy += vec2(0.0, size.y / 2.0);
    gl_Position = Transf * gl_Position;
    TexCoord = vec2(1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
void main( void )
{
  mat4 modelViewProj = ubo.proj * ubo.view * ubo.model;
    vec4 Position = gl_in[0].gl_Position;
    // Alpha: Aperture quad angle.
    float alpha = radians(-65.0) + 0.9*abs(sin(ubo.time));
    // Translate quad to origin
    mat4 Tgo = mat4(
	    vec4(1.0, 0.0, 0.0, 0.0),
	    vec4(0.0, 1.0, 0.0, 0.0),
	    vec4(0.0, 0.0, 1.0, 0.0),
	    vec4(-Position.xyz, 1.0));
    // Translate quad to original position
    mat4 Tini = mat4(
	    vec4(1.0, 0.0, 0.0, 0.0),
	    vec4(0.0, 1.0, 0.0, 0.0),
	    vec4(0.0, 0.0, 1.0, 0.0),
	    vec4(Position.xyz, 1.0));
    // Rotate quads with Y axis rotation. Give appearance of moving quads (left quad)
    mat4 RotQ = mat4(
	    vec4(cos(alpha), 0.0, -sin(alpha), 0.0),
	    vec4(0.0, 1.0, 0.0, 0.0),
	    vec4(sin(alpha), 0.0, cos(alpha), 0.0),
	    vec4(0.0, 0.0, 0.0, 1.0));
    // Rotate quads with Z axis rotation. "Flying" direction
    mat4 RotZ = mat4(
	    vec4(cos(ubo.beta), sin(ubo.beta), 0.0, 0.0),
	    vec4(-sin(ubo.beta), cos(ubo.beta), 0.0, 0.0),
	    vec4(0.0, 0.0, 1.0, 0.0),
	    vec4(0.0, 0.0, 0.0, 1.0));
    // Final transform matrix
    mat4 Transf = modelViewProj * RotZ * Tini * RotQ * Tgo;
    // Generate quad from original point´s position. Finally transform with Transf matrix.
    generateQuad(Transf, -1);
    // Alpha: Aperture angle for simetric quad
    alpha = PI - alpha;
    // Rotate quads with Y axis rotation. Give appearance of moving quads (right quad)
    RotQ = mat4(
	    vec4(cos(alpha), 0.0, -sin(alpha), 0.0),
	    vec4(0.0, 1.0, 0.0, 0.0),
	    vec4(sin(alpha), 0.0, cos(alpha), 0.0),
	    vec4(0.0, 0.0, 0.0, 1.0));
    Transf = modelViewProj * RotZ * Tini * RotQ * Tgo;
    generateQuad(Transf, 1);
}