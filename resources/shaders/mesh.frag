#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform ubo0
{
  mat4 model;
  mat4 view;
  mat4 proj;
	vec3 lightPos;
  vec3 lightColor;
	vec3 cameraPos;
};

layout( binding = 1 ) uniform sampler2D sColorMap;
layout( binding = 2 ) uniform sampler2D sNormalMap;

layout( location = 0 ) in vec3 outPosition;
layout( location = 1 ) in vec3 Normal;
layout( location = 2 ) in vec2 TexCoord;

layout( location = 0 ) out vec4 fragColor;

const float normalScale = 2.0;

vec3 perturb_normal(vec3 p, vec3 n)
{
  vec3 dp1 = dFdx(p);
  vec3 dp2 = dFdy(p);
  vec2 duv1 = dFdx(TexCoord);
  vec2 duv2 = dFdy(TexCoord);
  vec3 S = normalize(dp1 * duv2.t - dp2 * duv1.t);
  vec3 T = normalize(-dp1 * duv2.s + dp2 * duv1.s);
  vec3 N = normalize(n);
  vec3 mapN = texture(sNormalMap, TexCoord).xyz * 2.0 - 1.0;
  mapN.xy = normalScale * mapN.xy;
  mat3 tsn = mat3(S, T, N);
  return normalize(tsn * mapN);
}

void main( void )
{
  vec3 ambient = vec3(0.4);

  vec3 norm = normalize(Normal);
  norm = perturb_normal(-outPosition, norm);

  vec3 lightDir = normalize(lightPos - outPosition);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = lightColor * diff;

  vec3 viewDir = normalize(cameraPos - outPosition);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = vec3(spec);

  fragColor = vec4((ambient + diffuse + specular) * texture( sColorMap, TexCoord ).rgb, 1.0);
}