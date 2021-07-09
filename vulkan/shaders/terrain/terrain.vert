#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require


layout(location = 0) in vec4 position;
layout(location = 1) in uint normal;

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 worldSpacePosition;
layout(location = 2) out vec3 viewSpacePosition;
layout(location = 3) out vec4 intersectionPoint;
layout(location = 4) out float vMorph;

#include "../glsl_common.h"

layout(push_constant) uniform block
{
mat4 model;
vec4 intersection;
float morphFactor;
};

const float multiplier = 1.0f / 255.0f;

void main() 
{
    float height = position.y;//mix(position.y, position.w, morphFactor);
    vec4 worldSpace = model * vec4(position.x, height, position.z, 1.0);

    gl_Position = globalState.projection * globalState.view * worldSpace;

    float z = float(normal & 0xFF);
    float y = float((normal >> 8) & 0xFF);
    float x = float((normal >> 16) & 0xFF);

    vec3 norm = (vec3(x, y, z) * multiplier) * 2.0f - 1.0f;
    vnormal = inverse(transpose(mat3(model))) * norm;

    worldSpacePosition = worldSpace.xyz;
    viewSpacePosition = globalState.cameraPosition - worldSpacePosition.xyz;
    intersectionPoint = intersection;

    vMorph = morphFactor;
}
