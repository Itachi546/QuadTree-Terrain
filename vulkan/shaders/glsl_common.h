layout(set = 0, binding = 0) uniform GlobalState
{
	mat4 projection;
	mat4 view;
} globalState;

#ifdef USE_MODEL_MATRIX
layout(push_constant) uniform block
{
    mat4 model;
};
#endif

#ifdef VERTEX_MODE_VN
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 normal;
#endif
#ifdef VERTEX_MODE_VC
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 color;
#endif