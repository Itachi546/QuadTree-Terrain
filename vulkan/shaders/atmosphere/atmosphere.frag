#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#include "../shader_constants.h"

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 v_uv; 

layout(push_constant) uniform block
{
   mat4 u_invProjection;
   mat4 u_invView;
   vec4 cameraPosition;
};

layout(binding = 0) uniform sampler2D u_transmittance;
layout(binding = 1) uniform AtmosphereParams
{
  vec3 BetaR;
  float	EARTH_RADIUS;

  vec3	BetaM;
  float	ATMOSPHERE_RADIUS;

  float	Hr;
  float	Hm;
  int TRANSMITTANCE_TEXTURE_DIM;
};

#define SunColor              vec3(1.0, 1.0, 1.0)
#define SunIntensity          13.61839144264511
#define SunDiskMultiplier     10.0
#define SUN_DISTANCE          1.496e11
#define SUN_RADIUS            6.9551e8
const vec3 sunDir = normalize(vec3(0.0, -0.3, 1.0));

float intersectSphere(vec3 r0, vec3 rd, vec4 sph)
{
	vec3 rc = sph.xyz - r0;
	float radius2 = sph.w * sph.w;
	float tca = dot(rc, rd);
    //	if (tca < 0.) return;

	float d2 = dot(rc, rc) - tca * tca;
	if (d2 > radius2)
		return INF;

	float thc = sqrt(radius2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 < 0.) t0 = t1;
    return t0;
}

vec3 calculate_sun_disk(vec3 r0, vec3 rd)
{
    float threshold = asin(SUN_RADIUS / SUN_DISTANCE);
    float angle = acos(dot(rd, -sunDir));
    if(angle <= threshold * SunDiskMultiplier)
    {
        return SunIntensity * SunColor;
    }
    return vec3(0.0);
}

vec3 generate_direction(vec2 uv)
{
    vec4 d = vec4(uv, 0.0, 0.0);
    d = u_invProjection * d;
    d.z = -1.0f, d.w = 0.0f;

    vec3 worldSpace = vec3(u_invView * d);
    return normalize(worldSpace);
}

float phaseR(float cosTheta)
{
    return 3.0 / (16.0 * PI) * (1.0 + cosTheta * cosTheta);
}
float phaseM(float t)
{
    const float g = 0.76;
    float gg = g * g;
    float tt = t * t;
    float gt = g * t;
    
    float num = 3.0 * (1.0 - gg) * (1.0 + tt);
	float denom = (8.0 * PI) * (2.0 + gg) * pow(1.0 + gg - 2.0 * gt, 1.5);
    return num / denom;
}

vec4 get_optical_depth(vec3 p, vec3 rd)
{
    float height01 = length(p) - EARTH_RADIUS;
    float theta = 1 - (dot(normalize(p), rd) * .5 + .5);
    return texture(u_transmittance, vec2(theta, height01));
}



#define NUM_STEPS_LIGHT 16
vec2 light_march(vec3 r0, vec3 rd)
{
    float t = intersectSphere(r0, rd, vec4(0.0f, 0.0f, 0.0f, ATMOSPHERE_RADIUS));
    float stepSize = t / float(NUM_STEPS_LIGHT);

    float opticalDepthR = 0.0f;
    float opticalDepthM = 0.0f;

    vec3 p = r0 + EPSILON * rd;
    vec3 pInc = rd * stepSize;

    for(int i = 0; i < NUM_STEPS_LIGHT; ++i)
    {
       float h = length(p) - EARTH_RADIUS;
       //if(h < 0.0) 
       //	   break;
       opticalDepthR += exp(-h / Hr) * stepSize;
       opticalDepthM += exp(-h / Hm) * stepSize;

       p += pInc;
    }
    return vec2(opticalDepthR, opticalDepthM);
}

#define NUM_STEPS 64
vec3 calculate_scattering(vec3 r0, vec3 rd)
{
    float t = intersectSphere(r0, rd, vec4(0.0f, 0.0f, 0.0f, ATMOSPHERE_RADIUS));
    float stepSize = t / float(NUM_STEPS);

    float opticalDepthR = 0.0f;
    float opticalDepthM = 0.0f;

    vec3 scatteringR = vec3(0.0f);
    vec3 scatteringM = vec3(0.0f);

    float cosTheta = max(dot(rd, -sunDir), 0.0);
    vec3 p = r0 + EPSILON * rd;
    vec3 pInc = rd * stepSize;
    for(int i = 0; i < NUM_STEPS; ++i)
    {
       float h = length(p) - EARTH_RADIUS;
	   //if(h	< 0.0)
       //   break;
       float hr = exp(-h / Hr) * stepSize;
       float hm = exp(-h / Hm) * stepSize;

       opticalDepthR += hr;
       opticalDepthM += hm;

       vec2 opticalDepthL = light_march(p, -sunDir);

       vec3 rayleighScattering = (opticalDepthR + opticalDepthL.y) * BetaR;
       vec3 mieScattering = (opticalDepthM + opticalDepthL.y) * BetaM;
       scatteringR += exp(-rayleighScattering) * hr;
       scatteringM += exp(-mieScattering) * hm;
       p += pInc;
    }
   
   vec3 T = exp(-opticalDepthR * BetaR) + exp(-opticalDepthM * BetaM);
   vec3 L = calculate_sun_disk(r0, rd) * T;
   return L + (scatteringR * phaseR(cosTheta) * BetaR + 0.2f * scatteringM * phaseM(cosTheta) * BetaM) * SunIntensity;
}


void main() 
{
    vec3 r0 = cameraPosition.xyz;
    vec3 rd = generate_direction(v_uv);

	vec3 col = calculate_scattering(r0,	rd);
    //vec3 col = rd * 0.5f + 0.5f;
    col =col / (1.0 + col);
    fragColor = vec4(col, 1.0);
}
