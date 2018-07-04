#version 450

layout ( points ) in;
layout ( triangle_strip, max_vertices = 12 ) out;

layout(binding = 0) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 proj;
  float time;
};

mat4 rotationMatrix(vec3 axis, float angle)
{
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  float x = axis.x;
  float y = axis.y;
  float z = axis.z;
  return mat4(
        oc * x * x + c,  oc * x * y - z * s,  oc * z * x + y * s,  0.0,
    oc * x * y + z * s,      oc * y * y + c,  oc * y * z - x * s,  0.0,
    oc * z * x - y * s,  oc * y * z + x * s,      oc * z * z + c,  0.0,
                   0.0,                 0.0,                 0.0,  1.0
  );
}
const float GrassPatchSize = 5.0;
const float GrassPatchHeight = 3.5;
const float WindStrength = 0.5;
const vec3 WindDirection = vec3(1.0, 0.0, 0.5);

layout(location = 0) out vec2 uv;

void main( )
{
  mat4 mvp = proj * view * model;
  
  vec3 grassFieldPos = gl_in[0].gl_Position.xyz;

  float PIover180 = 3.1415/180.0;

  vec3 baseDir[3]=vec3[3](
    vec3(1.0, 0.0, 0.0),
    vec3(float(cos(45.0 * PIover180)), 0.0, float(sin(45.0 * PIover180))),
    vec3(float(cos(-45.0 * PIover180)), 0.0, float(sin(-45.0 * PIover180)))
  );
  for( int i = 0; i < 3; ++i )
  {
    vec3 baseDirRotated = (
       rotationMatrix(
          vec3(0.0, 1.0, 0.0), sin(time * 0.7) * 0.1)
      * vec4(baseDir[i], 1.0)).xyz;

    float WindPower = 0.5 +
      sin(grassFieldPos.x / 30.0 +
      grassFieldPos.z / 30.0 +
      time * (1.2 + WindStrength / 20.0) );

    if( WindPower < 0.0 )
    {
      WindPower = WindPower * 0.2;
    }
    else
    {
      WindPower = WindPower * 0.3;
    }
    WindPower *= WindStrength;

    // Grass patch top left vertex
    vec3 grassTL = grassFieldPos - baseDirRotated * GrassPatchSize * 0.5
      + WindDirection * WindPower;
    grassTL.y += GrassPatchHeight;
    gl_Position = mvp * vec4( grassTL, 1.0 );
    uv = vec2( 0.0, 0.0 );
    EmitVertex( );

    // Grass patch bottom left vertex
    vec3 grassBL = grassFieldPos - baseDir[i] * GrassPatchSize * 0.5;
    gl_Position = mvp * vec4( grassBL, 1.0 );
    uv = vec2( 0.0, 1.0 );
    EmitVertex( );

    // Grass patch top right vertex
    vec3 grassTR = grassFieldPos + baseDirRotated * GrassPatchSize * 0.5
      + WindDirection * WindPower;
    grassTR.y += GrassPatchHeight;
    gl_Position = mvp * vec4( grassTR, 1.0 );
    uv = vec2( 1.0, 0.0 );
    EmitVertex( );

    // Grass patch bottom right vertex
    vec3 grassBR = grassFieldPos + baseDir[i] * GrassPatchSize * 0.5;
    gl_Position = mvp * vec4( grassBR, 1.0 );
    uv = vec2( 1.0, 1.0 );
    EmitVertex( );

    EndPrimitive( );
  }
}