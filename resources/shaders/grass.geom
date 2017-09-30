#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

layout(binding = 0) uniform UniformBufferObject
{
  mat4 model;
  mat4 view;
  mat4 proj;
  float time;
} ubo;

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
const float GrassPatchSize = 3.0;
const float GrassPatchHeight = 1.5;
const float WindStrength = 1.0;
const vec3 WindDirection = vec3(1.0, 0.0, 1.0);

layout(location = 0) out vec2 TexCoord;

void main( void )
{
  mat4 mvp = ubo.proj * ubo.view * ubo.model;
  vec3 grassFieldPos = gl_in[0].gl_Position.xyz;
  /*const */float rad = radians( 45.0 );
  /*const */float sin45 = sin( rad );
  /*const */float cos45 = cos( rad );
  /*const */vec3 baseDir[3]=vec3[3](
    vec3(1.0, 0.0, 0.0),
    vec3(float(cos45), 0.0f, float(sin45)),
    vec3(float(cos45), 0.0f, float(sin45))
  );
  for(int i = 0; i < 3; ++i)
  {
    // Grass patch top left vertex
    vec3 baseDirRotated = (
       rotationMatrix(
          vec3(0.0, 1.0, 0.0), sin(ubo.time * 0.7) * 0.1)
      * vec4(baseDir[i], 1.0)).xyz;
    float WindPower = 0.5 +
      sin(grassFieldPos.x / 30.0 +
      grassFieldPos.z / 30.0 +
      ubo.time * (1.2 + WindStrength / 20.0) );
    if( WindPower < 0.0 )
    {
      WindPower = WindPower * 0.2;
    }
    else
    {
      WindPower = WindPower * 0.3;
    }
    WindPower *= WindStrength;
    vec3 grassTL = grassFieldPos - baseDirRotated * GrassPatchSize * 0.5
      + WindDirection * WindPower;
    grassTL.y += GrassPatchHeight;
    gl_Position = mvp * vec4(grassTL, 1.0);
    TexCoord = vec2(0.0, 1.0);
    EmitVertex();
    // Grass patch bottom left vertex
    vec3 grassBL = grassFieldPos - baseDir[i] * GrassPatchSize * 0.5;
    gl_Position = mvp * vec4(grassBL, 1.0);
    TexCoord = vec2(0.0, 0.0);
    EmitVertex();
    // Grass patch top right vertex
    vec3 grassTR = grassFieldPos + baseDirRotated * GrassPatchSize * 0.5
      + WindDirection * WindPower;
    grassTR.y += GrassPatchHeight;
    gl_Position = mvp * vec4(grassTR, 1.0);
    TexCoord = vec2(1.0, 1.0);
    EmitVertex();
    // Grass patch bottom right vertex
    vec3 grassBR = grassFieldPos + baseDir[i] * GrassPatchSize * 0.5;
    gl_Position = mvp * vec4(grassBR, 1.0);
    TexCoord = vec2(1.0, 0.0);
    EmitVertex();
    EndPrimitive();
  }
}