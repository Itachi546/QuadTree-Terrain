#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vnormal;

layout(location = 0) out vec4 fragColor;

void main() 
{
    vec3 normal = normalize(vnormal);
    vec3 start = vec3(0.2, 0.9, 0.1);
    vec3 end = vec3(0.4, 0.2, 0.1);

	vec3 albedo;
    if(normal.y > 0.98)
    {
        albedo = start;
    }
    else if(normal.y < 0.95)
    {
        albedo = end;
    }
    else
    {
        albedo = mix(end, start, 0.5f * (normal.y - 0.9) / 0.03);
    }

    vec3 col = vec3(0.0f);
    const vec3 ld = normalize(vec3(0.0, 1.0, 1.0));

    col += max(dot(normal, ld), 0.0) * vec3(1.28, 1.20, 0.99);
	col	+= (normal.y * 0.5 + 0.5) *	vec3(0.16, 0.20, 0.28);
    col /= (1.0 + col);
    fragColor = vec4(col, 1.0f);
}