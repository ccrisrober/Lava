#version 450

layout(location = 0) out vec2 outUV;

vec2 positions[4] = vec2[4](
    vec2(-0.5,-0.5),
    vec2(-0.5, 0.5),
    vec2( 0.5,-0.5),
    vec2( 0.5, 0.5)
);

vec2 texCoords[4] = vec2[4](
    vec2(0., 0.),
    vec2(0., 1.),
    vec2(1., 0.),
    vec2(1., 1.)
);

void main()
{
    outUV = texCoords[gl_VertexIndex];
    gl_Position = vec4(positions[gl_VertexIndex], 0., 1.);
}