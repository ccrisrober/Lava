#version 430
layout(location = 0) in vec3 outColor;
layout(location = 0) out vec4 fragColor;
void main()
{
  fragColor = vec4( outColor, 1.0 );
}