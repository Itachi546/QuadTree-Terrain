#pragma once

#include "core/base.h"
#include "core/ray.h"
#include "core/math.h"

class Context;
class Pipeline;
class VertexBuffer;
class OutlineFX;
class ShaderBindings;
class Entity;

struct Transform;


enum class Operation
{
	Translate,
	Rotate,
	Scale
};

class Gizmo
{
public:
	Gizmo() : m_width(0), m_height(0),
		projection(glm::mat4(1.0f)), view(glm::mat4(1.0f))
	{}

	void prepass(Context* context, ShaderBindings* globalBindings);
	void set_operation(Operation op) { operation = op; }
	void init(Context* context, int width, int height);
	void on_resize(uint32_t width, uint32_t height);

	void manipulate(glm::mat4 projection, glm::mat4 view, float mouseX, float mouseY, bool button);
	void render(Context* context);
	bool is_active() { return activeAxis != Axis::None; }
	void set_active(Entity* entity) { active = entity; }
	void destroy();

	Ray get_ray() { return m_ray; }
private:
	uint32_t m_width;
	uint32_t m_height;

	glm::mat4 projection, invProjection;
	glm::mat4 view, invView;

	enum class Axis
	{
		X = 0, Y, Z, None
	};
	Axis activeAxis = Axis::None;

	Operation operation = Operation::Translate;
	float distanceThreshold = 0.05f;
	// angle between view vector and the direction axis threshold
	const float minAngleThreshold = 0.95f;

	const glm::vec3 selectionColor = glm::vec3(1.0f, 0.51f, 0.06f);
	const glm::vec3 axisColor[3] = {
		glm::vec3(0.66f, 0.0f,  0.0f),
		glm::vec3(0.0f,  0.66f, 0.0f),
		glm::vec3(0.0f,  0.0f,  0.66f)
	};

	const glm::vec3 axes[3] = {
		glm::vec3(1.0f, 0.0f,  0.0f),
		glm::vec3(0.0f, 1.0f,  0.0f),
		glm::vec3(0.0f, 0.0f,  1.0f),
	};

	const float fixedFactor = 0.12f;
	Entity* active = nullptr;
	Ref<OutlineFX> m_outlineFX;
	glm::vec3 previous_intersect = glm::vec3(0.0f);
	Ray m_ray;
	//float prev_angle = 0.0f;

	bool prev_frame_button_state = false;

	glm::vec3 translate_start = glm::vec3(0.0f);

	void draw_translation();
	void manipulate_translation(const Ray& ray, bool button);

	void draw_rotation();
	void manipulate_rotation(const Ray& ray, bool button);

	void draw_scale();
	void manipulate_scale(const Ray& ray, bool button);
};