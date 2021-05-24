#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#define VERTEX_MODE_VN
#include "glsl_common.h"

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 worldSpacePosition;
layout(location = 2) out vec3 viewSpacePosition;
layout(location = 3) out vec4 intersectionPoint;

layout(push_constant) uniform block
{
mat4 model;
vec4 intersection;
};


void main() 
{
    vec4 worldSpace = model * vec4(position, 1.0);
    vec4 camSpace = globalState.view * worldSpace;
    gl_Position = globalState.projection * camSpace;

    vnormal = inverse(transpose(mat3(model))) * normal;
    worldSpacePosition = worldSpace.xyz;
    viewSpacePosition = camSpace.xyz;
    intersectionPoint = intersection;
}
