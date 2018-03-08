#version 450
#extension GL_ARB_separate_shader_objects : enable

// inputs
layout( location = 0 ) in vec3 vertTexCoord;

// descriptor sets
layout( set = 0, binding = 1 ) uniform samplerCube cubeMapSampler; 

// outputs
layout( location = 0 ) out vec4 outColor;
		
void main()	{
  outColor = texture( cubeMapSampler, vertTexCoord );
}