#include "shader_constants.h"

const bool enablePCF = true;
const bool enableShadowDebug = false;

struct LightProperties
{
	vec3 direction;
	float intensity;
	vec3 color;
	float castShadow;
};

struct Material
{
	vec3 albedo;
	float ao;
	float metallic;
	float roughness;
};

float distributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;

	float NdotH2 = NdotH * NdotH;

	float denom = NdotH2 * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;
	return a2 / denom;
}


float geometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float denom = NdotV * (1.0 - k) + k;
	return NdotV / denom;
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

#ifdef INCLUDE_IMPLEMENTATION
// Normal, Light and ViewDirection
vec3 calculateLight(in vec3 N, in LightProperties light, in Material material, in vec3 V)
{
	vec3 albedo = material.albedo;
	float roughness = material.roughness;
	float metallic = material.metallic;

	vec3 L = normalize(light.direction);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 H = normalize(L + V);
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float HdotV = max(dot(H, V), 0.0);

	float ambient = 0.1f;

	float D = distributionGGX(NdotH, roughness);
	float G = geometrySmith(NdotV, NdotL, roughness);
	vec3 F = fresnelSchlick(HdotV, F0);

	vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);

	vec3 kS = F;
	vec3 kD = (1.0f - kS) * (1.0 - metallic);

	float shadow = 1.0;
	if (light.castShadow > 0.5f)
	    shadow = calculateShadowFactor(worldSpacePosition, length(viewSpacePosition), enablePCF);

	vec3 radiance = light.color * light.intensity;
	vec3 color = (shadow * kD * albedo / PI + specular) * NdotL * radiance;
	return color;
}
#endif