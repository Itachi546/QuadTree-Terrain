#pragma once

#include "core/math.h"
#include <array>

class Camera
{
public:
	Camera();
	void update(float dt);
	glm::mat4 get_projection() const { return m_projection; }
	glm::mat4 get_view()       const { return m_view; }

	void set_position(glm::vec3 p)
	{
		m_position = p;
		m_targetPosition = p;
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

	std::array<glm::vec3, 8> get_frustum_corner()
	{
		return m_frustumPoints;
	}

	void rotate(float dx, float dy, float dt)
	{
		float m = m_sensitivity * dt;
		m_targetRotation.y += dy * m;
		m_targetRotation.x += dx * m;

		constexpr float maxAngle = glm::radians(89.99f);
		m_targetRotation.x = glm::clamp(m_targetRotation.x, -maxAngle, maxAngle);
	}

	glm::vec3 get_position() const { return m_position; }
private:
	glm::vec3 m_position;
	glm::vec3 m_rotation;

	glm::vec3 m_targetPosition;
	glm::vec3 m_targetRotation;

	glm::mat4 m_projection;
	float m_fov;
	float m_aspect;
	float m_nearPlane;
	float m_farPlane;

	glm::mat4 m_view;
	glm::vec3 m_target;
	glm::vec3 m_right;
	glm::vec3 m_up;
	glm::vec3 m_forward;
	float m_speed;
	float m_sensitivity;

	std::array<glm::vec3, 8> m_frustumPoints;

	void calculate_projection();
	void calculate_view();
};