#pragma once

#include "core/math.h"
#include "core/base.h"
#include "core/ray.h"
#include <array>

class Frustum;

class Camera
{
	friend class Scene;
public:
	Camera();

	// Don't forget to call update after changing camera parameter
	Ref<Camera> clone();

	void update(float dt);
	glm::mat4 get_projection() const { return m_projection; }
	glm::mat4 get_view()       const { return m_view; }

	glm::mat4 get_inv_projection() const { return m_invProjection; }
	glm::mat4 get_inv_view()       const { return m_invView; }

	void set_position(glm::vec3 p)
	{
		m_position = p;
		m_targetPosition = p;
	}

	void set_rotation(glm::vec3 rotation)
	{
		m_rotation = m_targetRotation = rotation;
	}

	glm::vec3 get_rotation()
	{
		return m_rotation;
	}

	void set_near_plane(float d)
	{
		m_nearPlane = d;
		calculate_projection();
	}

	void set_far_plane(float d)
	{
		m_farPlane = d;
		calculate_projection();
	}

	void set_height(float h)
	{
		m_targetPosition.y = h;
	}
	void set_aspect(float aspect) 
	{
		m_aspect = aspect;
		calculate_projection();
	}

	void set_fov(float fov)
	{
		m_fov = fov;
		calculate_projection();
	}

	float get_fov() { return m_fov; }

	float get_near_plane() { return m_nearPlane; }
	float get_far_plane() { return m_farPlane; }

	void walk(float dt)
	{
		m_targetPosition += m_speed * dt * m_forward;
	}

	void strafe(float dt)
	{
		m_targetPosition += m_speed * dt * m_right;
	}

	void lift(float dt)
	{
		m_targetPosition += m_speed * dt * m_up;
	}

	Ref<Frustum> get_frustum()
	{
		return m_frustum;
	}

	std::array<glm::vec3, 8> get_frustum_point(float nearPlane, float farPlane);


	void rotate(float dx, float dy, float dt)
	{
		float m = m_sensitivity * dt;
		m_targetRotation.y += dy * m;
		m_targetRotation.x += dx * m;

		constexpr float maxAngle = glm::radians(89.99f);
		m_targetRotation.x = glm::clamp(m_targetRotation.x, -maxAngle, maxAngle);
	}
	glm::vec3 get_forward() { return m_forward; }
	glm::vec3 get_position() const { return m_position; }

	Ray generate_ray(const glm::vec2& mouse, const glm::vec2& windowSize);
private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;

	glm::vec3 m_targetPosition;
	glm::vec3 m_targetRotation;

	glm::mat4 m_projection, m_invProjection;
	float m_fov;
	float m_aspect;
	float m_nearPlane;
	float m_farPlane;

	glm::mat4 m_view, m_invView;
	glm::vec3 m_target;
	glm::vec3 m_right;
	glm::vec3 m_up;
	glm::vec3 m_forward;
	float m_speed;
	float m_sensitivity;

	Ref<Frustum> m_frustum;

	void calculate_projection();
	void calculate_view();
};