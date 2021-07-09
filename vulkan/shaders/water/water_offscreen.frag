#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec3 viewSpacePosition;
layout(location = 0) out vec4 fragColor;

#include "../pbr.h"

layout(binding = 1) uniform Light
{
   LightProperties directionalLight;
};

layout(push_constant) uniform MaterialInput
{
    layout(offset = 64) Material material;
};


layout(binding = 2) uniform samplerCube u_cubemap;
layout(binding = 3) uniform samplerCube u_irradiance;

// Normal, Light and ViewDirection
vec3 calcLight(in vec3 N, in LightProperties light, in Material material, in vec3 V)
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
	vec3 radiance = light.color * light.intensity;
	vec3 color = (kD * albedo / PI + specular) * NdotL * radiance;
	return color;
}

void main() 
{
    vec3 N = normalize(vnormal);
    vec3 V = normalize(viewSpacePosition);

    vec3 col = calcLight(N, directionalLight, material, V);

    vec3 F0 = vec3(0.04f);
    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = (1.0 - kS) * (1.0 - material.metallic);
	vec3 irradiance = texture(u_irradiance, N).rgb;
    vec3 diffuse = irradiance * material.albedo;
    vec3 ambient = (kD * diffuse) * material.ao;
	col += ambient;

    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}