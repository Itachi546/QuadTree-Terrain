#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location	= 0) in	vec3 position;
layout(location	= 1) in	vec3 normal;

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 viewSpacePosition;

layout(push_constant) uniform block
{
   mat4 model;
   mat4 P;
   mat4 V;
   vec4 clipPlane;
};


void main() 
{
    vec4 worldSpacePosition = model * vec4(position, 1.0f);
    gl_ClipDistance[0] = dot(worldSpacePosition, clipPlane);

    vec4 viewSpace = V * worldSpacePosition;
    gl_Position = P * viewSpace;

    viewSpacePosition = viewSpace.xyz;
    vnormal = inverse(transpose(mat3(model))) * normal;
}