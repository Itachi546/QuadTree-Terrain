#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#define USE_MODEL_MATRIX
#define VERTEX_MODE_VN
#include "glsl_common.h"

layout(location = 0) out vec3 vnormal;

void main() {
    gl_Position = globalState.projection * globalState.view * model * vec4(position, 1.0);
    vnormal = inverse(transpose(mat3(model))) * normal;
}
