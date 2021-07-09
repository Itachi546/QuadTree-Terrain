#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#include "shader_constants.h"
#include "shadow/shadow.h"

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec3 worldSpacePosition;
layout(location = 2) in vec3 viewSpacePosition;

layout(location = 0) out vec4 fragColor;

#define INCLUDE_IMPLEMENTATION
#include "pbr.h"

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

void main() 
{

    vec3 N = normalize(vnormal);
    vec3 V = normalize(viewSpacePosition);

    vec3 col = calculateLight(N, directionalLight, material, V);

    vec3 F0 = vec3(0.04f);
    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = (1.0 - kS) * (1.0 - material.metallic);
	vec3 irradiance = texture(u_irradiance, N).rgb;
    vec3 diffuse = irradiance * material.albedo;
    vec3 ambient = (kD * diffuse) * material.ao;
	col += ambient;

	if(directionalLight.castShadow > 0.5f && enableShadowDebug)
	  col *= debugCascade();

    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}