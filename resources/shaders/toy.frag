#version 440

layout( location = 0 ) out vec4 fragColor;

layout( std140, set = 0, binding = 0 ) uniform ubo
{
	vec4 iMouse;
	vec4 iDate;
	vec4 iResolution;
	vec4 iChannelTime;
	vec4 iChannelResolution[ 4 ];

	float iGlobalDelta;
	float iGlobalFrame;
	float iGlobalTime;
	float iSampleRate;
};

/*layout( set = 1, binding = 0 ) uniform sampler2D iChannel0;
layout( set = 1, binding = 1 ) uniform sampler2D iChannel1;
layout( set = 1, binding = 2 ) uniform sampler2D iChannel2;
layout( set = 1, binding = 3 ) uniform sampler2D iChannel3;*/

#define iTime iGlobalTime

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 R = iResolution.xy;
    uv = ( fragCoord -.5*R ) / R.xy;
    uv.x *= iResolution.x/iResolution.y;
    float d = length(uv);
    float plt =0.5 + sin(iTime)/6.0;
    float plt2=0.06 + sin(iTime+3.14)/40.0;
    float r1 = 0.5;
    float r2 = 0.3;
    float c2= smoothstep(r2,r2-plt2,d);
    float c1 = smoothstep(r1,r1-plt,d);
    fragColor = vec4(vec3(c1-c2),1.0);

    float t = mod(3.*iTime,1.5)-1.;
    t = -20.*t*exp(-40.*t*t);
    
    vec2 p;
    float h;

       
    p=(8.+0.5*t)*(fragCoord/iResolution.xy-0.5);
    p.x*=iResolution.x/iResolution.y;
    p.y += 0.2;
    h = p.x*p.x+p.y*p.y-1.;
    h=h*h*h;
    h-=p.x*p.x*p.y*p.y*p.y;
    h=step(0.,h);  
   	fragColor = mix(vec4(1.,0.,0.,0.),fragColor,h);

}

void main( )
{
	mainImage( fragColor, vec2( gl_FragCoord.x, iResolution.y - gl_FragCoord.y ) );
}