#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#include "shadow.h"

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec3 worldSpacePosition;
layout(location = 2) in vec3 viewSpacePosition;
layout(location = 3) in vec4 intersectionPoint;

layout(location = 0) out vec4 fragColor;

const bool enablePCF = true;
const bool enableShadowDebug = false;

layout(binding = 1) uniform Light
{
   vec3 lightDirection;
   float lightIntensity;
   vec3 lightColor;
   float castShadow;
};

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
    vec3 normal = normalize(vnormal);
    vec3 col = vec3(0.0f);
	float shadow = 1.0f;

	if(castShadow > 0.5f)
	  shadow = calculateShadowFactor(worldSpacePosition, length(viewSpacePosition), enablePCF);

    col += max(dot(normal, lightDirection), 0.0) * lightColor * lightIntensity * shadow * calculateColor(normal);
	col	+= (normal.y * 0.5 + 0.5) *	vec3(0.16, 0.20, 0.28) * 0.5f;

	if(castShadow > 0.5f && enableShadowDebug)
	  col *= debugCascade();


	float fog = 1.0f - exp(-length(viewSpacePosition) * 0.002);
	col = mix(col, vec3(0.5, 0.7, 1.0), fog);

	if(intersectionPoint.w > 0.5f)
	{
	   const float radius = 10.0f;
	   float d = length(worldSpacePosition - intersectionPoint.xyz);
	   d = smoothstep(d, radius - 0.5f, radius);
	   col = mix(vec3(1.0f), col, d);
	}

    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}