#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(set = 0, binding = 0) uniform sampler2DArray tex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

void main( void )
{
	fragColor = texture(tex, vec3(uv, 0.0));

	if (uv.x > 0.5)
    {
    	if(uv.y < 0.5)
        {
			fragColor = texture(tex, vec3(uv, 0.0));
            //fragColor.rgb = vec3(1.0, 0.0, 0.0);
        }
        else
        {
			fragColor = texture(tex, vec3(uv, 1.0));
            //fragColor.rgb = vec3(0.0, 1.0, 0.0);
        }
    }
    else
    {
    	if(uv.y < 0.5)
        {
			fragColor = texture(tex, vec3(uv, 2.0));
            //fragColor.rgb = vec3(0.0, 0.0, 1.0);
        }
        else
        {
			fragColor = texture(tex, vec3(uv, 3.0));
            //fragColor.rgb = vec3(1.0, 1.0, 1.0);
        }
    }
    fragColor.a = 1.0;
}