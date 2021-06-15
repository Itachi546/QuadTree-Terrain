#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location	= 0) in	vec4 position;
layout(location	= 1) in	vec3 normal;

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 viewSpacePosition;

layout(push_constant) uniform block
{
   mat4 projection;
   mat4 view;
   vec4 clipPlane;
};


void main() 
{
    vec4 worldSpacePosition = vec4(position.xyz, 1.0f);
	gl_ClipDistance[0] = dot(worldSpacePosition, clipPlane);
    vec4 viewSpace = view * worldSpacePosition;

    gl_Position = projection * viewSpace;
    viewSpacePosition = viewSpace.xyz;
    vnormal = normal;
}
