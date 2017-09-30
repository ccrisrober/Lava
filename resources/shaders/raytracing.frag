#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform UniformBuffer
{
    float time;
} ubo;

float sphere(vec3 ray, vec3 dir, vec3 center, float radius)
{
	vec3 rc = ray-center;
	float c = dot(rc, rc) - (radius*radius);
	float b = dot(dir, rc);
	float d = b*b - c;
	float t = -b - sqrt(abs(d));
	float st = step(0.0, min(t,d));
	return mix(-1.0, t, st);
}

vec3 background(float t, vec3 rd)
{
	vec3 light = normalize(vec3(sin(t), 0.6, cos(t)));
	float sun = max(0.0, dot(rd, light));
	float sky = max(0.0, dot(rd, vec3(0.0, 1.0, 0.0)));
	float ground = max(0.0, -dot(rd, vec3(0.0, 1.0, 0.0)));
	return 
		(pow(sun, 256.0)+0.2*pow(sun, 2.0))*vec3(2.0, 1.6, 1.0) +
		pow(ground, 0.5)*vec3(0.4, 0.3, 0.2) +
		pow(sky, 1.0)*vec3(0.5, 0.6, 0.7);
}

void mainImage( out vec4 fragColor, in vec2 uv )
{
	vec3 ro = vec3(0.0, 0.0, -3.0);
	vec3 rd = normalize(vec3(uv, 1.0));
	vec3 p = vec3(0.0, 0.0, 0.0);
	float t = sphere(ro, rd, p, 1.0);
	vec3 nml = normalize(p - (ro+rd*t));
	vec3 bgCol = background(ubo.time, rd);
	rd = reflect(rd, nml);
	vec3 col = background(ubo.time, rd) * vec3(0.9, 0.8, 1.0);
	fragColor = vec4( mix(bgCol, col, step(0.0, t)), 1.0 );
}

void main( void )
{
	vec2 old_uv = uv;
	old_uv.y = 1.0 - old_uv.y;
	vec2 new_uv = -1.0 + 2.0 * old_uv;
  	mainImage( fragColor, new_uv );
}