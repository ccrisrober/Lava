#version 450

layout( location = 0 ) in vec3 pos;
layout( location = 1 ) in vec3 normal;
layout( location = 2 ) flat in int GIsEdge;
layout( location = 0 ) out vec4 fragColor;

const vec3 LineColor = vec3( 0.0, 0.0, 0.0 );

const vec3 LightPosition = vec3( 0.0, 0.0, 0.0 );
const vec3 LightIntensity = vec3( 1.0, 1.0, 1.0 );

const vec3 Ka = vec3( 0.2, 0.2, 0.2 );
const vec3 Kd = vec3( 0.7, 0.5, 0.2 );

const int levels = 5;
const float scaleFactor = 1.0 / levels;

void main( )
{
  if( GIsEdge == 1 )
  {
    fragColor = vec4( LineColor, 1.0 );
  }
  else
  {
  	vec3 s = normalize( LightPosition.xyz - pos.xyz );
    vec3 ambient = Ka;
    float cosine = dot( s, normal );
    vec3 diffuse = Kd * ceil( cosine * levels ) * scaleFactor;

    fragColor =  vec4( LightIntensity * (ambient + diffuse), 1.0 );
  }
}