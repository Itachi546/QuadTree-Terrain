#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location	= 0) in	vec3 position;
layout(location	= 1) in	vec3 normal;


layout(set = 0, binding = 0) uniform GlobalState
{
	mat4 projection;
	mat4 view;
	vec3 cameraPosition;
} globalState;

layout(push_constant) uniform block
{
    mat4 model;
};


layout(location = 0) out vec3 vnormal;
layout(location = 1) out mat4 VP;

void main() 
{
    VP = globalState.projection * globalState.view;
    gl_Position = model * vec4(position, 1.0);
    vnormal = inverse(transpose(mat3(model))) * normal;
}
