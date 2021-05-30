#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#define VERTEX_MODE_VN
#include "glsl_common.h"
layout(push_constant) uniform block
{
    mat4 model;
    mat4 lightVP;
};

void main() {
    gl_Position = lightVP * model * vec4(position, 1.0);
    gl_Position.z = max(gl_Position.z, 0.0f);
}
