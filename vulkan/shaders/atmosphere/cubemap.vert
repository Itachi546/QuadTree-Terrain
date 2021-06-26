#version 430
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(push_constant) uniform Matrices
{
   mat4 u_projection;
   mat4 u_view;
   vec4 cameraPosition;
   vec4 sunDirection;
};

layout(location = 0) out vec3 uv;
layout(location = 1) out vec3 rayOrigin;
layout(location = 2) out vec4 sunDir;
layout(location = 3) out vec3 rayDirection;

void main()
{
    vec4 clipPos = u_projection * u_view * vec4(position, 0.0f);
    gl_Position = clipPos.xyww;

    uv = position;

    rayOrigin = cameraPosition.xyz;
    rayDirection = normalize(position);
    sunDir = sunDirection;
}
