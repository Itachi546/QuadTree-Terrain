#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#include "shadow.h"

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec3 worldSpacePosition;
layout(location = 2) in vec3 viewSpacePosition;

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

void main() 
{
    vec3 normal = normalize(vnormal);
    vec3 col = vec3(0.0f);
	float shadow = 1.0f;
	if(castShadow > 0.5f)
	  shadow = calculateShadowFactor(worldSpacePosition, length(viewSpacePosition), enablePCF);

    col += max(dot(normal, normalize(lightDirection)), 0.0) * lightColor * lightIntensity * shadow;
	col	+= (normal.y * 0.5 + 0.5) *	vec3(0.16, 0.20, 0.28);

	
	float fog = 1.0f - exp(-length(viewSpacePosition) * 0.002);
	col = mix(col, vec3(0.5, 0.7, 1.0), fog);

	if(castShadow > 0.5f && enableShadowDebug)
	  col *= debugCascade();
    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}