#include "../shader_constants.h"

#define NUM_SAMPLES 32
#define NUM_SAMPLES_LIGHT 16

#define EARTH_RADIUS 6.360e6
#define ATMOSPHERE_RADIUS 6.42e6
#define SunDiskMultiplier     10.0
#define SunIntensity          13.61839144264511
#define SUN_DISTANCE          1.496e11
// 695,510 km = (6.9551 * 10^8) m
#define SUN_RADIUS            6.9551e8

#define Hr                    7.994e3
#define Hm                    1.2e3
#define BetaR                 vec3(5.8e-6, 13.5e-6, 33.1e-6)
#define BetaM                 vec3(21e-6)

vec3 sunDir = normalize(vec3(0.0, -0.5, 0.0));
vec3 sunImage(vec3 r0, vec3 rd, vec3 sunDir)
{
    // EARTH_RADIUS is too small compared to SUN_DISTANCE
    //float denom = exp(0.5 * (log(SUN_DISTANCE + EARTH_RADIUS) + log(SUN_DISTANCE - EARTH_RADIUS)));
    
    float threshold = asin(SUN_RADIUS / SUN_DISTANCE);
    float angle = acos(dot(rd, -sunDir));
    if(angle <= threshold * SunDiskMultiplier)
    {
        return vec3(SunIntensity);
    }
    return vec3(0.0);
}

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

/*
vec3 generate_direction(vec2 uv)
{
    vec4 d = vec4(uv, 0.0, 0.0);
    d = u_invProjection * d;
    d.z = -1.0f;
    d.w = 0.0f;

    vec3 worldSpace = vec3(u_invView * d);
    return normalize(worldSpace);
}
*/

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
/*
vec4 get_optical_depth(vec3 p, vec3 rd)
{
    float height01 = length(p) - EARTH_RADIUS;
    float theta = 1.0 - (dot(normalize(p), rd) * .5 + .5);
    return texture(u_transmittance, vec2(theta, height01));
}
*/



#define NUM_STEPS_LIGHT 16
vec2 light_march(vec3 r0, vec3 rd)
{
    float t = intersectSphere(r0, rd, vec4(0.0f, 0.0f, 0.0f, ATMOSPHERE_RADIUS));
    float stepSize = t / float(NUM_STEPS_LIGHT);

    float opticalDepthR = 0.0f;
    float opticalDepthM = 0.0f;

    vec3 p = r0 + EPSILON * rd;
    vec3 pInc = rd * EPSILON;
    for(int i = 0; i < NUM_STEPS_LIGHT; ++i)
    {
       float h = length(p) - EARTH_RADIUS;
       if(h < 0.0)
            break;
       opticalDepthR += exp(-h / Hr) * stepSize;
       opticalDepthM += exp(-h / Hm) * stepSize;

       p += pInc;
    }
    return vec2(opticalDepthR, opticalDepthM);
}

#define NUM_STEPS 32
vec3 calculate_scattering(vec3 r0, vec3 rd)
{
    float t = intersectSphere(r0, rd, vec4(0.0f, 0.0f, 0.0f, ATMOSPHERE_RADIUS));
    float stepSize = t / float(NUM_STEPS);

    float opticalDepthR = 0.0f;
    float opticalDepthM = 0.0f;

    vec3 scattering = vec3(0.0f);

    float cosTheta = max(dot(rd, -sunDir), 0.0);
    vec3 p = r0 + EPSILON * rd;
    vec3 pInc = rd * stepSize;

    float pR = phaseR(cosTheta);
    float pM = phaseM(cosTheta);

    for(int i = 0; i < NUM_STEPS; ++i)
    {
       float h = length(p) - EARTH_RADIUS;
       if(h < 0.0)
            break;
       float hr = exp(-h / Hr) * stepSize;
       float hm = exp(-h / Hm) * stepSize;
       
       opticalDepthR += hr;
       opticalDepthM += hm;

       vec2 opticalDepthL = light_march(p, -sunDir);

       vec3 rayleighScattering = exp(-(opticalDepthR + opticalDepthL.y) * BetaR);
       vec3 mieScattering = exp(-(opticalDepthM + opticalDepthL.y) * BetaM);
       scattering += (rayleighScattering * hr * pR * BetaR + 1.1 * mieScattering * hm * pM * BetaM);
       p += pInc;
    }

     // Just magic number
   vec3 T = exp(-opticalDepthR * BetaR - opticalDepthM * BetaM);
   return scattering * 20.0 + sunImage(r0, rd, -sunDir) * T;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;
    vec3 r0 = vec3(0.0, EARTH_RADIUS + 10.0, 0.0);
    vec3 tgt = r0 + vec3(0.0, 0.0, 1.0);
    vec3 ww = normalize(tgt - r0);
    vec3 uu = normalize(cross(vec3(0, 1, 0), ww));
    vec3 vv = normalize(cross(ww, uu));
    vec3 rd = normalize(uv.x * uu + uv.y * vv + ww);
/*
    float angle_min = 45.0 * (PI / 180.0);
    float angle_max = 95.0 * (PI / 180.0);
    float t = fract(iTime * 0.1);
    t = (t <= 0.5)
        ? (angle_min + (angle_max - angle_min) * t * 2.0)
        : (angle_max + (angle_min - angle_max) * (t - 0.5) * 2.0);
    sunDir = vec3(0.0, -cos(t), sin(t));

    vec3 col = calculate_scattering(r0, rd);
    */
    vec3 col = normalize(rd);
    col /= (1.0 + col);
    col = pow(col, vec3(0.4545));
    fragColor = vec4(col, 1.0);
}

