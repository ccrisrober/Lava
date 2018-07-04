#version 450

layout (location = 0) in vec2 gDistance;
layout (location = 1) in vec3 gNormal;

layout (location = 0) out vec4 fragColor;

const float Scale = 20.0;
const float Offset = -1.0;

const vec3 LightPosition = vec3(0.25, 0.25, 1.0);
const vec3 AmbientMaterial = vec3(0.04, 0.04, 0.04);
const vec3 SpecularMaterial = vec3(0.5, 0.5, 0.5);
const vec3 FrontMaterial = vec3(0.75, 0.75, 0.5);
const vec3 BackMaterial = vec3(0.5, 0.5, 0.75);

const float Shininess = 50.0;

vec4 amplify(float d, vec3 color)
{
    d = Scale * d + Offset + gl_FragCoord.z;
    d = clamp(d, 0.0, 1.0);
    d = 1.0 - exp2(-2.0 * d * d);
    return vec4(d * color, 1.0);
}

void main( )
{
    vec3 N = normalize(gNormal);
    
    if (!gl_FrontFacing)
    {
        N = -N;
    }
        
    vec3 L = normalize(LightPosition);
    vec3 Eye = vec3( 0.0, 0.0, 1.0 );
    vec3 H = normalize(L + Eye);
    
    float df = max(0.0, dot(N, L));
    float sf = max(0.0, dot(N, H));
    sf = pow(sf, Shininess);

    vec3 color = gl_FrontFacing ? FrontMaterial : BackMaterial;
    vec3 lighting = AmbientMaterial + df * color;

    if (gl_FrontFacing)
    {
        lighting += sf * SpecularMaterial;
    }

    float d = min(gDistance.x, gDistance.y);
    fragColor = amplify(d, lighting);
}