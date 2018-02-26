#version 450
#extension GL_ARB_separate_shader_objects : enable

// vertex attributes
layout(location = 0) in float fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4( fragColor );
}