#include "gizmo.h"
#include "debug_draw.h"
#include "renderer/context.h"
#include "renderer/pipeline.h"
#include "common/common.h"
#include "renderer/device.h"
#include "scene/entity.h"
#include "fx/outlinefx.h"
#include "imgui/imgui.h"

#include <algorithm>

static Ray generate_ray(glm::mat4 inv_view, glm::mat4 inv_projection, float mouseX, float mouseY, float width, float height)
{
	// clip space direction
	glm::vec4 d = glm::vec4((mouseX / width) * 2.0f - 1.0f, 1.0f - 2.0f * (mouseY / height), 0.0f, 0.0f);

	// camera space direction
	d = inv_projection * d;
	d.z = -1.0f;
	d.w = 0.0f;

	// world space direction
	glm::vec3 worldSpace = glm::vec3(inv_view * d);
	return Ray{ glm::vec3(inv_view[3]), glm::normalize(worldSpace) };
}

static float closest_distance(const Ray& l1, const Ray& l2, float& t)
{
	// l1 = p1 + t1 * v1
	// l2 = p2 * t2 * v2
	float v1sqr = glm::dot(l1.direction, l1.direction);
	float v2sqr = glm::dot(l2.direction, l2.direction);
	float v1v2 = glm::dot(l1.direction, l2.direction);
	glm::vec3 dp = l2.origin - l1.origin;
	float det = v1sqr * v2sqr - v1v2 * v1v2;
	if (det > FLT_MIN)
	{
		float dpv1 = glm::dot(l1.direction, dp);
		float dpv2 = glm::dot(l2.direction, dp);

		float invDet = 1.0f / det;
		float t1 = invDet * (v2sqr * dpv1 - v1v2 * dpv2);
		float t2 = invDet * (v1v2 * dpv1 - v1sqr * dpv2);

		t = t1;
		glm::vec3 d = (l2.origin + t2 * l2.direction) - (l1.origin + t1 * l1.direction);
		return glm::dot(d, d);
	}
	else
	{
		glm::vec3 a = glm::cross(l2.origin - l1.origin, l1.direction);
		return std::sqrt(dot(a, a) / v1sqr);
	}
}

static float closest_distance(const Ray& ray, glm::vec3 normal, glm::vec3 center, float radius, glm::vec3& p)
{
	float denom = glm::dot(ray.direction, normal);
	if (abs(denom) > FLT_MIN)
	{
		float t = glm::dot(center - ray.origin, normal) / denom;
		p = ray.origin + t * ray.direction;
		glm::vec3 circumferencePoint = center + radius * glm::normalize(p - center);
		return glm::length(circumferencePoint - p);
	}
	return FLT_MAX;
}

void Gizmo::prepass(Context* context, ShaderBindings* globalBindings)
{
	if (active != nullptr)
	{
		m_outlineFX->render(context, { active }, globalBindings);
	}
}

void Gizmo::init(Context* context, int width, int height)
{
	m_width = width;
	m_height = height;
	m_outlineFX = CreateRef<OutlineFX>();
	m_outlineFX->init(width, height);

	PipelineDescription pipelineDesc = {};
	pipelineDesc.renderPass = context->get_global_renderpass();

	std::string vertexCode = load_file("spirv/line.vert.spv");
	std::string fragmentCode = load_file("spirv/line.frag.spv");

	ShaderDescription lineShaders[2] = {
		ShaderDescription{ShaderStage::Vertex,   vertexCode,   static_cast<uint32_t>(vertexCode.size())},
		ShaderDescription{ShaderStage::Fragment, fragmentCode, static_cast<uint32_t>(fragmentCode.size())},
	};
	pipelineDesc.shaderStageCount = 2;
	pipelineDesc.shaderStages = lineShaders;
	pipelineDesc.rasterizationState.faceCulling = FaceCulling::None;
	pipelineDesc.rasterizationState.topology = Topology::Line;
	pipelineDesc.rasterizationState.lineWidth = 1.0f;
	pipelineDesc.rasterizationState.enableDepthTest = false;
	pipelineDesc.rasterizationState.enableDepthWrite = false;
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;

}

void Gizmo::on_resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	m_outlineFX->resize(width, height);
}

void Gizmo::manipulate(glm::mat4 projection, glm::mat4 view, float mouseX, float mouseY, bool button)
{
	this->projection = projection;
	this->view = view;
	this->invProjection = glm::inverse(projection);
	this->invView = glm::inverse(view);

	render_ui();

	if (active == nullptr || !m_enableGizmo)
		return;

	if (button == false)
		activeAxis = Axis::None;



	Ray ray = generate_ray(invView, invProjection, mouseX, mouseY, float(m_width), float(m_height));
	switch (operation)
	{

	case Operation::Translate:
		manipulate_translation(ray, button);
		draw_translation();
		break;
	case Operation::Rotate:
		manipulate_rotation(ray, button);
		draw_rotation();
		break;
	case Operation::Scale:
		manipulate_scale(ray, button);
		draw_scale();
		break;
	}

	prev_frame_button_state = button;
}

void Gizmo::render(Context* context)
{
	if(active != nullptr && m_enableGizmo)
		DebugDraw::immediate_draw_textured_quad(context, m_outlineFX->get_output_image_bindings());
}

void Gizmo::render_ui()
{
	if (ImGui::CollapsingHeader("Gizmo", 0, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Checkbox("Enabled", &m_enableGizmo);
		const char* items[] = { "Translate", "Rotate", "Scale" };
		static int currentItem = (int)operation;
		if (ImGui::Combo("Operation", &currentItem, items, IM_ARRAYSIZE(items)))
			operation = Operation(currentItem);
	}
}

void Gizmo::destroy()
{
	m_outlineFX->destroy();
}


void Gizmo::draw_translation()
{
	// camera space position
	glm::vec3 camPos = invView[3];
	glm::vec3 origin = active->transform->position;
	float distance = glm::distance(camPos, origin);
	float scale = fixedFactor * distance;
	glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);

	for (int i = 0; i < 3; ++i)
	{
		if (std::abs(forward[i]) > minAngleThreshold)
			continue;
		glm::vec3 color = axisColor[i];
		if (int(activeAxis) == i)
			color = selectionColor;

		DebugDraw::draw_line_no_depth(origin, origin + scale * axes[i], color, 4);

		glm::vec3 start = origin + scale * 0.9f * axes[i];
		glm::vec3 right = glm::normalize(glm::cross(forward, axes[i]));
		glm::vec3 v0 = start + glm::vec3(0.06f * scale) * right;
		glm::vec3 v1 = start - glm::vec3(0.06f * scale) * right;
		glm::vec3 v2 = start + 0.12f * scale * axes[i];

		DebugDraw::draw_triangle(v0, v1, v2, color);
	}

	if (activeAxis != Axis::None && prev_frame_button_state)
	{
		float distance = glm::distance(camPos, translate_start);
		const float radius = 0.05f * distance* fixedFactor;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), translate_start); 
		model *= glm::mat4(glm::mat3(invView));
		model = glm::scale(model, glm::vec3(radius * 0.5f));
		DebugDraw::draw_circle_no_depth(model, glm::vec3(1.0f));

		glm::vec3 dir = glm::normalize(origin - translate_start);
		DebugDraw::draw_line_no_depth(translate_start + (radius * 0.5f + 0.005f) * dir, origin, glm::vec3(1.0f), 2);
	}

}

void Gizmo::manipulate_translation(const Ray& ray, bool button)
{
	glm::vec3 origin = active->transform->position;
	float distance_to_camera = glm::distance(ray.origin, origin);
	float scale = fixedFactor * distance_to_camera;

	Ray axes_ray[3] = {
		Ray{origin, axes[0]},
		Ray{origin, axes[1]},
		Ray{origin, axes[2]},
	};

	float t[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
	float axis_distance[3] = { FLT_MAX, FLT_MAX, FLT_MAX };

	glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);
	if (activeAxis == Axis::None)
	{
		for (int i = 0; i < 3; ++i)
		{
			if(std::abs(forward[i]) < minAngleThreshold)
				axis_distance[i] = closest_distance(ray, axes_ray[i], t[i]);
		}

		float d = std::min(axis_distance[0], std::min(axis_distance[1], axis_distance[2]));
		if (d < distanceThreshold)
		{
			if (d == axis_distance[0])
				activeAxis = Axis::X;
			else if (d == axis_distance[1])
				activeAxis = Axis::Y;
			else if(d == axis_distance[2])
				activeAxis = Axis::Z;

			int index = int(activeAxis);
			glm::vec3 p = ray.origin + t[index] * ray.direction;
			float delta = p[index] - origin[index];
			if ((delta < 0.0f || delta > scale || t[index] < 0.0f))
				activeAxis = Axis::None;
		}
	}
	else
	{
		int index = int(activeAxis);
		closest_distance(ray, axes_ray[index], t[index]);
	}


	if (activeAxis != Axis::None)
	{
		int index = int(activeAxis);
		glm::vec3 p = ray.origin + t[index] * ray.direction;

		if (prev_frame_button_state)
		{
			float delta = p[index] - origin[index];

			if (button)
				active->transform->position[index] += p[index] - previous_intersect[index];
		}

		previous_intersect = p;
	}

	if (prev_frame_button_state == false && button)
		translate_start = active->transform->position;

}

void Gizmo::draw_rotation()
{
	// camera space position
	glm::vec3 camPos = invView[3];
	glm::vec3 origin = active->transform->position;
	float distance = glm::distance(camPos, origin);

	float radius = distance * fixedFactor;
	
	// Z-Axis
	glm::mat4 translate = glm::translate(glm::mat4(1.0f), origin);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(radius));

	glm::mat4 model = translate * scale;
	DebugDraw::draw_circle_no_depth(model, activeAxis == Axis::Z ? selectionColor : axisColor[2]);

	// Y-Axis;
	model = translate * glm::rotate(glm::mat4(1.0f), float(PI_2), glm::vec3(1.0f, 0.0f, 0.0f)) * scale;
	DebugDraw::draw_circle_no_depth(model, activeAxis == Axis::Y ? selectionColor : axisColor[1]);

	// X-Axis
	model = translate * glm::rotate(glm::mat4(1.0f), float(PI_2), glm::vec3(0.0f, 1.0f, 0.0f)) * scale;
	DebugDraw::draw_circle_no_depth(model, activeAxis == Axis::X ? selectionColor : axisColor[0]);

	// Bounding Circle
	scale = glm::scale(glm::mat4(1.0f), glm::vec3(radius * 1.1f));
	model = translate * glm::mat4(glm::mat3(invView)) * scale;
	DebugDraw::draw_circle_no_depth(model, glm::vec3(1.0f));

}

void Gizmo::manipulate_rotation(const Ray& ray, bool button)
{
	glm::vec3 camPos = invView[3];
	glm::vec3 origin = active->transform->position;
	float distance = glm::distance(camPos, origin);
	float radius = distance * fixedFactor;

	glm::vec3 points[3] = { };
	float axis_distance[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
	if (activeAxis == Axis::None)
	{
		for (int i = 0; i < 3; ++i)
			axis_distance[i] = closest_distance(ray, axes[i], origin, radius, points[i]);

		float d = std::min(axis_distance[0], std::min(axis_distance[1], axis_distance[2]));
		if (d < distanceThreshold)
		{
			if (d == axis_distance[0])
				activeAxis = Axis::X;
			else if (d == axis_distance[1])
				activeAxis = Axis::Y;
			else if (d == axis_distance[2])
				activeAxis = Axis::Z;
		}
	}
	else
	{
		int index = int(activeAxis);
		closest_distance(ray, axes[index], origin, radius, points[index]);
	}


	if (activeAxis != Axis::None)
	{

		int index = int(activeAxis);
		glm::vec3 p = points[index];
		if (prev_frame_button_state)
		{
			glm::vec3 axis = axes[index];

			float ang = glm::clamp(glm::dot(glm::normalize(previous_intersect), glm::normalize(p)), -1.0f, 1.0f);
			glm::vec3 forward = glm::normalize(origin - camPos);
			glm::vec3 perp = glm::cross(previous_intersect, p);

			float v = glm::dot(perp, axis);
			if (std::abs(v) > FLT_MIN)
				v /= std::abs(v);

			if (button)
				active->transform->rotation = glm::angleAxis(acos(ang) * 2.0f * v, axis) * active->transform->rotation;
		}
		previous_intersect = p;
	}

	if (prev_frame_button_state == false && button)
		translate_start = active->transform->position;


}

void Gizmo::draw_scale()
{
	// camera space position
	glm::vec3 camPos = invView[3];
	glm::vec3 origin = active->transform->position;
	float distance = glm::distance(camPos, origin);

	float scale = fixedFactor * distance;
	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 color = int(activeAxis) == i ? selectionColor : axisColor[i];
		glm::vec3 end = origin + scale * axes[i];
		DebugDraw::draw_line_no_depth(origin, end, color, 4);
		DebugDraw::draw_points_no_depth(end, color);
	}
}

void Gizmo::manipulate_scale(const Ray& ray, bool button)
{
	glm::vec3 origin = active->transform->position;
	float distance_to_camera = glm::distance(ray.origin, origin);
	float scale = fixedFactor * distance_to_camera;

	Ray axes_ray[3] = {
		Ray{origin, axes[0]},
		Ray{origin, axes[1]},
		Ray{origin, axes[2]},
	};

	float t[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
	float axis_distance[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
	if (activeAxis == Axis::None)
	{
		for (int i = 0; i < 3; ++i)
			axis_distance[i] = closest_distance(ray, axes_ray[i], t[i]);

		float d = std::min(axis_distance[0], std::min(axis_distance[1], axis_distance[2]));
		if (d < distanceThreshold)
		{
			if (d == axis_distance[0])
				activeAxis = Axis::X;
			else if (d == axis_distance[1])
				activeAxis = Axis::Y;
			else if (d == axis_distance[2])
				activeAxis = Axis::Z;

			int index = int(activeAxis);
			glm::vec3 p = ray.origin + t[index] * ray.direction;
			float delta = p[index] - origin[index];
			if ((delta < 0.0f || delta > scale || t[index] < 0.0f))
				activeAxis = Axis::None;
		}
	}
	else
	{
		int index = int(activeAxis);
		closest_distance(ray, axes_ray[index], t[index]);
	}


	if (activeAxis != Axis::None)
	{
		int index = int(activeAxis);
		glm::vec3 p = ray.origin + t[index] * ray.direction;

		if (prev_frame_button_state)
		{
			float delta = p[index] - origin[index];

			if (button)
				active->transform->scale[index] += p[index] - previous_intersect[index];
		}
		previous_intersect = p;
	}
}
