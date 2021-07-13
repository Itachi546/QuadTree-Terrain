#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#include "../shader_constants.h"
#include "../shadow/shadow.h"

layout(location = 0) in vec3 vnormal;
layout(location = 1) in vec2 vuv;
layout(location = 2) in float noise;
layout(location = 3) in vec3 fviewDir;

layout(location = 0) out vec4 fragColor;

#include "../pbr.h"

layout(binding = 1) uniform Light
{
    LightProperties directionalLight;
};
layout(binding = 2) uniform samplerCube u_cubemap;
layout(binding = 3) uniform samplerCube u_irradiance;


vec3 gbColor = vec3(0.01, 0.5, 0.01);
vec3 gtColor = vec3(0.1, 0.9, 0.1);

void main() 
{

    vec3 N = normalize(vnormal);
    vec3 L = normalize(directionalLight.direction);
    vec3 V = normalize(fviewDir);
    vec3 H = normalize(L + V);
    
    float specular = pow(max(dot(N, H), 0.0), 16.0) * 3.0;

    vec3 ao = texture(u_irradiance, N).rgb;
    vec3 col = mix(gbColor, gtColor, vuv.y * noise * 2.0f) * (ao) * 2.;

    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}