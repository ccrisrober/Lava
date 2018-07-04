#version 450

layout( location = 0 ) out vec3 outColor;

layout( binding = 1 ) uniform ubo
{
    mat4 transform;
};

vec3 colors[3] = vec3[](
    vec3( 1.0, 0.0, 0.0 ),
    vec3( 0.0, 0.0, 1.0 ),
    vec3( 0.0, 1.0, 0.0 )
);

void main( )
{
    float L = 1.0;
    float y = sqrt(3.0 * L) / 4.0;
    vec2 v[3] = vec2[](
        vec2(   0.0, -y ),
        vec2( -L/2.0, y ),
        vec2(  L/2.0, y )
    );
    vec2 c = (v[0] + v[1] + v[2])/3.;
    vec2 pos = v[gl_VertexIndex] - c;
    float scale = 1.7;
    outColor = colors[gl_VertexIndex];
    gl_Position = transform * vec4(pos * scale, 0.0, 1.0);
}