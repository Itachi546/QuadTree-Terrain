#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D image;

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv; 

void main() 
{
    fragColor = texture(image, v_uv);
}
