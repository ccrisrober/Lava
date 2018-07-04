#version 450

layout (location = 0) in vec3 outNormal;

layout (location = 0) out vec4 fragColor;

const vec3 LightPosition = vec3(1.0, 0.0, 0.0);
const vec3 AmbientMaterial = vec3( 0.1, 0.18725, 0.1745 );//vec3(0.04, 0.04, 0.04);
const vec3 SpecularMaterial = vec3( 0.297254, 0.30829, 0.306678 );//vec3(0.5, 0.5, 0.5);
const vec3 FrontMaterial = vec3( 0.396, 0.74151, 0.69102 );//vec3(1.0, 0.62, 0.51);
const vec3 BackMaterial = FrontMaterial; //vec3(0.5, 0.5, 0.75);
const float Shininess = 0.1;

const float A = 0.1;
const float B = 0.3;
const float C = 0.6;
const float D = 1.0;

void main( )
{
	vec3 N = normalize(outNormal);
    if (!gl_FrontFacing)
    {
	 	N = -N;
    }

    vec3 L = normalize(LightPosition);
    vec3 Eye = vec3(0.0, 0.0, 1.0);
    vec3 H = normalize(L + Eye);
    
    float df = max(0.0, dot(N, L));
    float E = fwidth(df);
    if (df > A - E && df < A + E)
    {
        df = mix(A, B, smoothstep(A - E, A + E, df));
    }
    else if (df > B - E && df < B + E)
    {
        df = mix(B, C, smoothstep(B - E, B + E, df));
    }
    else if (df > C - E && df < C + E)
    {
        df = mix(C, D, smoothstep(C - E, C + E, df));
    }
    else if (df < A)
    {
    	df = 0.0;
    }
    else if (df < B)
    {
    	df = B;
    }
    else if (df < C)
    {
    	df = C;
    }
    else
    {
    	df = D;
    }

    float sf = max(0.0, dot(N, H));
    sf = pow(sf, Shininess);
    E = fwidth(sf);
    if (sf > 0.5 - E && sf < 0.5 + E)
    {
        sf = clamp(0.5 * (sf - 0.5 + E) / E, 0.0, 1.0);
    }
    else
    {
        sf = step(0.5, sf);
    }

    vec3 color = gl_FrontFacing ? FrontMaterial : BackMaterial;
    vec3 lighting = AmbientMaterial + df * color;
    if (gl_FrontFacing)
    {
        lighting += sf * SpecularMaterial;
    }

    fragColor = vec4(lighting, 1.0);
}