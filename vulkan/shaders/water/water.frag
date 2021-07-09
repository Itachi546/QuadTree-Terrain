#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec3 viewDirection;
layout(location = 2) in vec4 clipSpacePosition;
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

layout(binding = 4) uniform sampler2D reflectionTexture;
layout(binding = 5) uniform sampler2D refractionTexture;
layout(binding = 6) uniform sampler2D refractionDepthTexture;

layout(binding = 7) uniform WaterParams
{
   vec3 waterColor;
   float maxDepth;

   vec3 foamColor;
   float maxFoamDepth;

   float zN;
   float zF;

   float shoreBlendDistance;
};


//const vec3 foamColor =  vec3(1.0f);
//const float maxFoamDepth = 10.0f;


vec3 calculate_light(vec3 viewDir, vec3 lightDir, vec3 normal)
{
   vec3 reflection = normalize(reflect(-lightDir, normal));
   float direction = max(0.0, dot(viewDir, reflection));
   vec3 col = pow(direction, 16.0) * lightColor * lightIntensity;
   col += max(dot(lightDir, normal), 0.0) * lightColor * waterColor.xyz * lightIntensity;
   //col += vec3(0.1f);
   return col;
}

void main() 
{ 
   const float	zRange = zF	- zN;

   vec3 viewDir = normalize(viewDirection);
   vec3 lightDir = normalize(lightDirection);
   vec3 normal = normalize(vnormal);

   float dist = length(viewDirection);
   vec2 uv = (clipSpacePosition.xy / clipSpacePosition.w) * 0.5f + 0.5f;
   float depth = texture(refractionDepthTexture, vec2(uv.x, 1.0 - uv.y)).r;
   float floorDistance = (2.0 * zN * zF) / (zF + zN - depth * zRange);
   float curDepth = gl_FragCoord.z;
   float waterDistance = (2.0 * zN * zF) / (zF + zN - curDepth * zRange);

   float waterDepth = max(floorDistance - waterDistance, 0.0);
   float alpha = clamp(waterDepth / shoreBlendDistance, 0.0, 1.0);

   float distortionFactor = max(dist * 0.5f, 10.0f);
   vec2 distortion = normal.xz / distortionFactor;
   vec3 refl = texture(reflectionTexture, uv + distortion).rgb;
   vec3 refr = texture(refractionTexture, vec2(uv.x, 1.0 - uv.y) - distortion).rgb;
   float absorption	= clamp(waterDepth / maxDepth, 0.0, 1.0);
   refr = mix(refr, waterColor.xyz, absorption); 

   float cos_theta = max(dot(viewDir, vec3(0.0, 1.0, 0.0)), 0.0);
   float f0 = 0.02;
   float reflectance = f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
   vec3 col = calculate_light(viewDir, lightDir, normal) * 0.4 + refl;
   col = mix(refr, col, reflectance);
   col = mix(foamColor, col, smoothstep(0.0, maxFoamDepth, waterDepth));
   col /=(1.0f + col);


   fragColor = vec4(col, alpha);
}