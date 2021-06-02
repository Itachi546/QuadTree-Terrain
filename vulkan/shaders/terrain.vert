#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 worldSpacePosition;
layout(location = 2) out vec3 viewSpacePosition;
layout(location = 3) out vec4 intersectionPoint;
layout(location = 4) out float vMorph;
#include "glsl_common.h"

layout(push_constant) uniform block
{
mat4 model;
vec4 intersection;
float morphFactor;
};


void main() 
{
    float y = position.y;
    vec4 worldSpace = model * vec4(position.x, y, position.z, 1.0);
    vec4 camSpace = globalState.view * worldSpace;
    gl_Position = globalState.projection * camSpace;

    vnormal = inverse(transpose(mat3(model))) * normal;
    worldSpacePosition = worldSpace.xyz;
    viewSpacePosition = camSpace.xyz;
    intersectionPoint = intersection;

    vMorph = morphFactor;
}
