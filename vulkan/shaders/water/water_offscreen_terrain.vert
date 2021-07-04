#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive   : require

layout(location	= 0) in	vec4 position;
layout(location	= 1) in	uint normal;

layout(location = 0) out vec3 vnormal;
layout(location = 1) out vec3 viewSpacePosition;

layout(push_constant) uniform block
{
   mat4 projection;
   mat4 view;
   vec4 clipPlane;
   vec4 cameraPosition;
};

const float multiplier = 1.0f / 255.0f;

void main() 
{
    vec4 worldSpacePosition = vec4(position.xyz, 1.0f);
	gl_ClipDistance[0] = dot(worldSpacePosition, clipPlane);
    vec4 viewSpace = view * worldSpacePosition;

    gl_Position = projection * viewSpace;
    viewSpacePosition = cameraPosition.xyz - worldSpacePosition.xyz;

    float z = float(normal & 0xFF);
    float y = float((normal >> 8) & 0xFF);
    float x = float((normal >> 16) & 0xFF);
    vnormal = (vec3(x, y, z) * multiplier) * 2.0f - 1.0f;

}
