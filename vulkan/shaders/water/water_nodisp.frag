#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location = 0) in vec2 vuv;
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

layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D reflectionTexture;
layout(binding = 5) uniform sampler2D refractionTexture;
layout(binding = 6) uniform sampler2D refractionDepthTexture;

const vec3 waterColor =  vec3(0.1, 0.3, 0.7);
const vec3 foamColor =  vec3(1.0f);

const float zN = 0.01f;
const float zF = 2000.0f;
const float maxFoamDepth = 0.05f;
const float zRange = zF - zN;

vec3 calculate_light(vec3 viewDir, vec3 lightDir, vec3 normal)
{
   vec3 reflection = normalize(reflect(-lightDir, normal));
   float direction = max(0.0, dot(viewDir, reflection));
   vec3 col = pow(direction, 16.0) * lightColor * lightIntensity;
   col += max(dot(lightDir, normal), 0.0) * lightColor * waterColor * lightIntensity;
   //col += vec3(0.1f);
   return col;
}

void main() 
{ 
   vec3 viewDir = normalize(viewDirection);
   vec3 lightDir = normalize(lightDirection);
   float dist = length(viewDirection);
   vec3 normal = vec3(0.0f, 1.0f, 0.0f);
   normal = normalize(texture(normalMap, vuv).rgb);


   vec2 uv = (clipSpacePosition.xy / clipSpacePosition.w) * 0.5f + 0.5f;
   float depth = texture(refractionDepthTexture, vec2(uv.x, 1.0 - uv.y)).r;
   float floorDistance = (2.0 * zN * zF) / (zF + zN - depth * zRange);
   float curDepth = gl_FragCoord.z;
   float waterDistance = (2.0 * zN * zF) / (zF + zN - curDepth * zRange);

   float waterDepth = max(floorDistance - waterDistance, 0.0);
   float alpha = clamp(waterDepth * 0.25f, 0.0, 1.0);

   float distortionFactor = max(dist * 0.5f, 10.0f);
   vec2 distortion = (normal.xz / distortionFactor) * 0.5f;
   vec3 refl = texture(reflectionTexture, uv + distortion).rgb;
   vec3 refr = texture(refractionTexture, vec2(uv.x, 1.0 - uv.y) - distortion).rgb;
   float absorption	= clamp(waterDepth / 20.0f, 0.0, 1.0);
   refr = mix(refr, waterColor, absorption); 

   float cos_theta = max(dot(viewDir, vec3(0.0, 1.0, 0.0)), 0.0);
   float f0 = 0.02;
   float reflectance = f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
   vec3 col = calculate_light(viewDir, lightDir, normal) + refl;
   col = mix(refr, col, reflectance);

   col = mix(foamColor,	col, smoothstep(0.0, maxFoamDepth, waterDepth));
   col /=(1.0f + col);


   fragColor = vec4(col, 1.0);
}