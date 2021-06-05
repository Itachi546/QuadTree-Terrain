#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 position[] = vec2[6](
vec2(-1.0f, -1.0f),
vec2( 1.0f, -1.0f),
vec2( 1.0f,  1.0f),
vec2( 1.0f,  1.0f),
vec2(-1.0f,  1.0f),
vec2(-1.0f, -1.0f)
);

layout(location = 0) out vec2 v_uv; 

layout(push_constant) uniform block
{
    vec4 translate;
};

void main() 
{
    vec2 pos = position[gl_VertexIndex];
    gl_Position = vec4(translate.xy + pos * translate.zw, 0.0f, 1.0f);
	v_uv = pos * 0.5f + 0.5f;
}
