#version 430

#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform samplerCube skybox;

layout(location = 0) in vec3 uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) in vec3 rayOrigin;
layout(location = 2) in vec4 sunDirection;
layout(location = 3) in vec3 rayDirection;

#define SunColor              vec3(1.28, 1.20, 0.99)
#define SunIntensity          13.61839144264511
#define SunDiskMultiplier     5.0
#define SUN_DISTANCE          1.496e11
#define SUN_RADIUS            6.9551e8

vec3 calculate_sun_disk(vec3 r0, vec3 rd, vec3 sunDir, float intensity)
{
    float threshold = asin(SUN_RADIUS / SUN_DISTANCE);
    float angle = acos(dot(rd, sunDir));
    if(angle <= threshold * SunDiskMultiplier)
    {
        return SunIntensity * SunColor * intensity;
    }
    return vec3(0.0);
}


void main()
{
   vec3 L = calculate_sun_disk(rayOrigin, normalize(rayDirection), normalize(sunDirection.xyz), sunDirection.w);
   L += texture(skybox, uv).rgb;
   L /=(1.0 + L);
   fragColor = vec4(L, 1.0f);
}
