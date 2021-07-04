#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#define USE_MODEL_MATRIX
#define VERTEX_MODE_VN
#include "glsl_common.h"

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 worldSpacePosition;
layout(location = 2) out vec3 viewSpacePosition;
void main() 
{
    vec4 worldSpace = model * vec4(position, 1.0);
    gl_Position = globalState.projection * globalState.view * worldSpace;

    vnormal = inverse(transpose(mat3(model))) * normal;
    worldSpacePosition = worldSpace.xyz;

     viewSpacePosition = globalState.cameraPosition - worldSpacePosition.xyz;
    //viewSpacePosition = camSpace.xyz;
}
