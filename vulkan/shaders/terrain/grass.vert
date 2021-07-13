#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#define USE_MODEL_MATRIX
#include "../glsl_common.h"
layout(location = 0) in vec4 position;
layout(location = 1) in uint normal;


layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 viewDir;
layout(location = 2) out mat4 VP;

const float multiplier = 1.0f / 255.0f;

void main() 
{
    VP = globalState.projection * globalState.view;
    gl_Position = model * vec4(position.xyz, 1.0);
    float z = float(normal & 0xFF);
    float y = float((normal >> 8) & 0xFF);
    float x = float((normal >> 16) & 0xFF);

    vec3 norm = (vec3(x, y, z) * multiplier) * 2.0f - 1.0f;
    vnormal = inverse(transpose(mat3(model))) * norm;
    viewDir = globalState.cameraPosition - position.xyz;
}
