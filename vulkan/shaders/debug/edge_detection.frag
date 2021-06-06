#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D image;

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv; 

layout(push_constant) uniform block
{
    vec2 inv_resolution;
};

const int SAMPLE_COUNT = 2;
void main() 
{
    vec3 col = vec3(0.0f);
    for(int i = 0; i < SAMPLE_COUNT; ++i)
    {
        for(int j = 0; j < SAMPLE_COUNT; ++j)
        {
            vec2 offset = vec2(i * 2, j * 2) * inv_resolution;
            col += texture(image, v_uv + offset).rgb; 
            col += texture(image, v_uv - offset).rgb; 
        }
    }

    col /= float(SAMPLE_COUNT * SAMPLE_COUNT);

    float luma = dot(col, vec3(0.2126, 0.7152, 0.0722));
    fragColor = (luma > 0.1f && luma < 0.9f)? vec4(1.0f, 0.51f, 0.06f, 1.0f) : vec4(0.0f);
}
