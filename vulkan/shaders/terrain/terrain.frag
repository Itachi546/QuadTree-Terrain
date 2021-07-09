#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#include "../shadow/shadow.h"

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec3 worldSpacePosition;
layout(location = 2) in vec3 viewSpacePosition;
layout(location = 3) in vec4 intersectionPoint;
layout(location = 4) in float vMorph;

layout(location = 0) out vec4 fragColor;

#define INCLUDE_IMPLEMENTATION
#include "../pbr.h"


layout(binding = 1) uniform Light
{
   LightProperties directionalLight;
};
layout(binding = 2) uniform samplerCube u_cubemap;
layout(binding = 3) uniform samplerCube u_irradiance;



vec3 grassColor = vec3(0.01f, 0.5f, 0.01f);
vec3 rockColor = vec3(0.4f, 0.2f, 0.01f);
vec3 calculateColor(vec3 n)
{
    vec3 col = grassColor;
    float slope	= 1.0f - n.y;
	if(slope < 0.2f)
	{
	   return grassColor;
	}
	if(slope < 0.7f && slope >= 0.2f)
	{
      float blendAmount = (slope - 0.2f) / (0.7f - 0.2f);
	  col = mix(grassColor, rockColor, blendAmount);
	}
	if(slope >= 0.7f)
	   col = rockColor;

	return col;
}

void main() 
{

    vec3 N = normalize(vnormal);
    vec3 V = normalize(viewSpacePosition);

    Material material;
    material.albedo = calculateColor(N);
    material.roughness = 1.0;
    material.ao = 0.5;
    material.metallic = 0.0;

    vec3 col = calculateLight(N, directionalLight, material, V);

    vec3 F0 = vec3(0.04f);
    vec3 kS = F0;//fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = (1.0 - kS);
    kD *=(1.0 - material.metallic);

	vec3 irradiance = texture(u_irradiance, N).rgb;
    vec3 diffuse = irradiance * material.albedo;
    vec3 ambient = (kD * diffuse) * material.ao;
	col += ambient;

	if(directionalLight.castShadow > 0.5f && enableShadowDebug)
	  col *= debugCascade();

    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}
