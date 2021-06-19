#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location = 0) in vec3 position;

layout(location = 0) out vec2 vuv;
layout(location = 1) out vec3 viewDirection;
layout(location = 2) out vec4 clipSpacePosition;

layout(set = 0, binding = 0) uniform GlobalState
{
	mat4 projection;
	mat4 view;
} globalState;

layout(push_constant) uniform block
{
    vec4 translate;
    vec4 cameraPosition;
};
layout(binding = 2) uniform sampler2D displacementMap;

const float frequency = 1.0f / 128.0f;

void main() 
{
    vec4 worldSpace = vec4(position + translate.xyz, 1.0);
    vec4 camSpace = globalState.view * worldSpace;
    clipSpacePosition = globalState.projection * camSpace;
    gl_Position = clipSpacePosition;

    viewDirection = cameraPosition.xyz - worldSpace.xyz;
    vuv = position.xz * frequency;
}