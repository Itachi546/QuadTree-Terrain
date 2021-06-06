#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 worldSpacePosition;
layout(location = 2) out vec3 viewSpacePosition;

layout(set = 0, binding = 0) uniform GlobalState
{
	mat4 projection;
	mat4 view;
} globalState;

layout(push_constant) uniform block
{
    vec3 translate;
};

layout(binding = 2) uniform sampler2D displacementMap;
layout(binding = 3) uniform sampler2D normalMap;

const float frequency = 1.0f / 256.0f;

void main() 
{
    vec2 uv = position.xz * frequency;
    float height = texture(displacementMap, uv).r;
    vec4 worldSpace = vec4(position.x + translate.x, height + translate.y, position.z + translate.z, 1.0);
    vec4 camSpace = globalState.view * worldSpace;
    gl_Position = globalState.projection * camSpace;

    vnormal = texture(normalMap, uv).rgb;
    worldSpacePosition = worldSpace.xyz;
    viewSpacePosition = camSpace.xyz;
}
