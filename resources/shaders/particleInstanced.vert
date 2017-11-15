#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(binding = 1) uniform UniformBufferObject2
{
    float Time;
    vec3 Gravity;	// in world coords
    float ParticleLifeTime;
};

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTexCoord;
layout (location = 3) in vec3 VertexInitialVelocity;	// in world coords
layout (location = 4) in float StartTime;

layout (location = 0) out vec3 Position;
layout (location = 1) out vec3 Normal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    // Work in camera coordinates
    Position = (view * model * vec4(VertexPosition,1.0)).xyz;
    mat3 NormalMatrix = mat3(inverse(transpose(view * model)));
    Normal = NormalMatrix * VertexNormal;
    vec3 g = NormalMatrix * Gravity;
    vec3 v0 = NormalMatrix * VertexInitialVelocity;

    if( Time > StartTime )
    {
        float t = Time - StartTime;

        if( t < ParticleLifetime )
        {
            Position += v0 * t + Gravity * t * t;
        }
    }

    // Draw at the current position
    gl_Position = ProjectionMatrix * vec4(Position, 1.0);
}