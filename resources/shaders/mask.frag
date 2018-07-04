#version 450

layout( early_fragment_tests ) in;

layout( binding = 0 ) uniform sampler2D _MainTex;
layout( binding = 1 ) uniform sampler2D _MaskTex;
layout( binding = 2 ) uniform sampler2D _TransTex;

layout( push_constant ) uniform PushConsts
{
  float _MaskValue;
};

layout( location = 0 ) in vec2 uv;
layout( location = 0 ) out vec4 fragColor;

void main( )
{
  vec4 col = texture( _MainTex, uv );
  vec4 mask = texture( _MaskTex, uv );
  vec4 trans = texture( _TransTex, uv );

  if( mask.b > _MaskValue )
  {
    fragColor = trans;
  }
  else
  {
    fragColor = col;
  }
}