#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

layout (location = 0) out vec4 fragColor;

const vec4 lightPosition = vec4(0.0, 0.0, 0.0, 1.0);
const vec3 lightIntensity = vec3( 1.0, 1.0, 1.0 );
const vec3 Ka = vec3( 0.1, 0.1, 0.1 );
const vec3 Kd = vec3( 0.9, 0.5, 0.2 );
const vec3 Ks = vec3( 0.95, 0.95, 0.95 );
const float Shininess = 100.0;

vec3 iluminatti( )
{
    vec3 s = normalize(vec3(lightPosition) - Position);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect( -s, Normal );
    vec3 ambient = lightIntensity * Ka;
    float sDotN = max( dot(s, Normal), 0.0 );
    vec3 diffuse = lightIntensity * Kd * sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
    {
        spec = lightIntensity * Ks * pow( max( dot(r, v), 0.0 ), Shininess );
    }

    return ambient + diffuse + spec;
}

void main()
{
    fragColor = vec4( iluminatti( ), 1.0);
}