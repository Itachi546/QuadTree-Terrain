#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

#define VERTEX_MODE_VC
#include "glsl_common.h"

layout(location = 0) out vec3 vcolor;
layout(push_constant) uniform block
{
    mat4 model;
    vec3 tintColor;
};

void main() {
    gl_Position = globalState.projection * globalState.view * model * vec4(position, 1.0);
	gl_PointSize = 10.0;
    vcolor = color * tintColor;
}
