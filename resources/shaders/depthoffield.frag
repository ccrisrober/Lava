#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D colorTex;
layout(binding = 1) uniform sampler2D vertexTex;

layout(binding = 2) uniform UniformBufferObject
{
  float fD;
  float mD;
} ubo;

layout(early_fragment_tests) in;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

//const float fD = 10.0;
//const float mD = 5.0;
const float near = 0.1;
const float far = 10.0;

#define MASK_SIZE_ 25u
const vec2 texIdx_[MASK_SIZE_] = vec2[](
  vec2(-2.0, 2.0), vec2(-1.0, 2.0), vec2( 0.0, 2.0), vec2( 1.0, 2.0), vec2( 2.0, 2.0),
  vec2(-2.0, 1.0), vec2(-1.0, 1.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0), vec2( 2.0, 1.0),
  vec2(-2.0, 0.0), vec2(-1.0, 0.0), vec2( 0.0, 0.0), vec2( 1.0, 0.0), vec2( 2.0, 0.0),
  vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2( 0.0,-1.0), vec2( 1.0,-1.0), vec2( 2.0,-1.0),
  vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2( 0.0,-2.0), vec2( 1.0,-2.0), vec2( 2.0,-2.0));

const float maskFactor = float (1.0/65.0);
const float mask_[MASK_SIZE_] = float[](
  1.0 * maskFactor, 2.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor, 1.0 * maskFactor,
  2.0 * maskFactor, 3.0 * maskFactor, 4.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor,
  3.0 * maskFactor, 4.0 * maskFactor, 5.0 * maskFactor, 4.0 * maskFactor, 3.0 * maskFactor,
  2.0 * maskFactor, 3.0 * maskFactor, 4.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor,
  1.0 * maskFactor, 2.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor, 1.0 * maskFactor);

float LinearizeDepth( float depth )
{
  float z = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));  
}

void main( )
{
  // TODO: Move to an uniform/push cte variable
  vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0));
  float d = LinearizeDepth(texture(vertexTex, uv).z);
  float dof = abs(d -ubo.fD) * ubo.mD;
  dof = clamp (dof, 0.0, 1.0);
  dof *= dof;
  vec4 color = vec4 (0.0);
  for (uint i = 0u; i < MASK_SIZE_; ++i)
  {
    vec2 iidx = uv + ts * texIdx_[i] * dof;
    color += texture(colorTex, iidx, 0.0) * mask_[i];
  }
  fragColor = color; 
}