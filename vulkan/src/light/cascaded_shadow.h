#pragma once

#include "core/base.h"
#include "core/math.h"
#include <array>

class Framebuffer;
class Camera;
class RenderPass;
class Framebuffer;
class Pipeline;
class Camera;
class Context;
class Scene;
class ShaderBindings;
class UniformBuffer;

class ShadowCascade
{
public:
	ShadowCascade(const glm::vec3& direction);

	void update(Ref<Camera> camera);
	void render(Context* context, Scene* scene, bool renderShadow = true);
	void destroy();

	void set_light_direction(const glm::vec3& dir)
	{
		m_direction = glm::normalize(dir);
	}

	glm::vec3 get_light_direction()
	{
		return m_direction;
	}

	ShaderBindings* get_depth_bindings() { return m_bindings; }
private:

	struct Cascade
	{
		glm::mat4 VP;
		// matching to the layout of shader
		glm::vec4 splitDepth;
	};

	static const int CASCADE_COUNT = 4;
	const int SHADOW_MAP_DIMENSION = 2048;
	float m_cascadeSplits[CASCADE_COUNT];
	std::array<Cascade, CASCADE_COUNT> m_cascades;
	Framebuffer* m_cascadeFramebuffer[CASCADE_COUNT];


	float cascadeSplitLambda = 0.85f;
	float shadowDistance = 150.0f;
	float nearDistance = 0.01f;

	glm::vec3 m_direction;

	ShaderBindings* m_bindings;
	Pipeline* m_pipeline;
	RenderPass* m_renderPass;
	UniformBuffer* m_ubo;

	void render_ui();
	void calculate_split_distance();
};