struct Cascade
{
	mat4 lightViewProjection;
	vec4 splitDepth;
};

layout(set = 0, binding = 2) uniform sampler2D depthMap0;
layout(set = 0, binding = 3) uniform sampler2D depthMap1;
layout(set = 0, binding = 4) uniform sampler2D depthMap2;
layout(set = 0, binding = 5) uniform sampler2D depthMap3;
layout(set = 0, binding = 6) uniform shadowMapData
{
	Cascade cascades[4];
};

// bias matrix is column major 
//x` = x * 0.5 + 0.5
//y` = y * 0.5 + 0.5
//z` = z
//const mat4 biasMat = mat4( 
//	0.5, 0.0, 0.0, 0.0,
//	0.0, 0.5, 0.0, 0.0,
//	0.0, 0.0, 1.0, 0.0,
//	0.5, 0.5, 0.0, 1.0 
//);

// @TODO Better Shadow Quality
// https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-17-efficient-soft-edged-shadows-using

const float bias = 0.01f;
const float scale = 0.75f;

float get_shadow(sampler2D depthTexture, vec4 projectedCoord, vec2 duv)
{
	vec2 uv = vec2(projectedCoord.x + duv.x, (1.0f - projectedCoord.y) + duv.y);
	float depth = texture(depthTexture, uv).r;
	return (depth >= projectedCoord.z - bias) ? 1.0 : 0.0;
}

float getSplitShadowValue(sampler2D depthTexture, int index, vec3 p, bool enablePCF)
{
	vec4 projectedCoord = (cascades[index].lightViewProjection) * vec4(p, 1.0f);
	projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	projectedCoord.xyz /= projectedCoord.w;

	float shadow = 0.0;
	if (enablePCF)
	{
		vec2 texSize = textureSize(depthTexture, 0).xy;
		float	dx = scale * 1.0f / texSize.x;
		float	dy = scale * 1.0f / texSize.y;
		int range = 2;
		int count = 0;
		for (int i = -range; i <= range; ++i)
		{
			for (int j = -range; j <= range; ++j)
			{
				shadow += get_shadow(depthTexture, projectedCoord, vec2(i * dx, j * dy));
				count++;
			}
		}
		shadow = shadow / count;
	}
	else
	{
		shadow += get_shadow(depthTexture, projectedCoord, vec2(0.0));
	}
	return shadow;
}

int cascadeIndex = 0;
float calculateShadowFactor(vec3 p, float distanceFromCamera, bool enablePCF)
{
	cascadeIndex = 0;
	for (int i = 0; i < 4; ++i) {
		if (distanceFromCamera > cascades[i].splitDepth.x)
			cascadeIndex = i + 1;
	}
	switch (cascadeIndex)
	{
	case 0:
		return getSplitShadowValue(depthMap0, cascadeIndex, p, enablePCF);
	case 1:
		return getSplitShadowValue(depthMap1, cascadeIndex, p, enablePCF);
	case 2:
		return getSplitShadowValue(depthMap2, cascadeIndex, p, enablePCF);
	case 3:
		return getSplitShadowValue(depthMap3, cascadeIndex, p, enablePCF);
	default:
		return 1.0;
	}
	return 1.0;
}


vec3 debugCascade()
{
	switch (cascadeIndex) {
	case 0:
		return vec3(1.0f, 0.25f, 0.25f);
	case 1:
		return vec3(0.25f, 1.0f, 0.25f);
	case 2:
		return vec3(0.25f, 0.25f, 1.0f);
		break;
	case 3:
		return vec3(1.0f, 1.0f, 0.25f);
		break;
	}
	return vec3(1.0f);
}