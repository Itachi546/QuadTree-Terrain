#include "camera.h"

Camera::Camera()
{
	m_targetPosition = m_position = glm::vec3(0.0f, 1.0f, -3.0f);
	m_targetRotation = m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	m_fov = glm::radians(60.0f);
	m_aspect = 4.0f / 3.0f;
	m_nearPlane = 0.01f;
	m_farPlane = 1000.0f;

	m_target = glm::vec3(0.0f, 0.0f, 1.0f);
	m_speed = 2.0f;
	m_sensitivity = 0.8f;

	calculate_projection();
	calculate_view();
}

void Camera::update(float dt)
{
#if 1
	m_position += (m_targetPosition - m_position) * 0.99f * dt * 2.0f;
	m_rotation += (m_targetRotation - m_rotation) * 0.99f * dt * 2.0f;

	if (glm::length2(m_targetPosition - m_position) < 0.00001f)
		m_position = m_targetPosition;
	if (glm::length2(m_targetRotation - m_rotation) < 0.0000001f)
		m_rotation = m_targetRotation;

#else
	m_position = m_targetPosition;
	m_rotation = m_targetRotation;
#endif
	calculate_view();

	// Calculate frustum corner
	static const glm::vec3 frustumCorners[8] =
	{
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
	};

	// Project frustum corners into world space
	// inv(view) * inv(projection) * p
	// inv(A) * inv(B) = inv(BA)
	glm::mat4 invCam = glm::inverse(m_projection * m_view);
	for (uint32_t i = 0; i < 8; i++) {
		glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
		m_frustumPoints[i] = invCorner / invCorner.w;
	}
}

void Camera::calculate_projection()
{
	m_projection = glm::perspective(m_fov, m_aspect, m_nearPlane, m_farPlane);
}

void Camera::calculate_view()
{
	glm::mat3 rotate = glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z);
	glm::vec3 up = glm::normalize(rotate * vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 forward = glm::normalize(rotate * vec3(0.0f, 0.0f, 1.0f));
	m_view = glm::lookAt(m_position, m_position + forward , up);

	m_right   = glm::vec3(m_view[0][0], m_view[1][0], m_view[2][0]);
	m_up      = glm::vec3(m_view[0][1], m_view[1][1], m_view[2][1]);
	m_forward = glm::vec3(m_view[0][2], m_view[1][2], m_view[2][2]);
}
