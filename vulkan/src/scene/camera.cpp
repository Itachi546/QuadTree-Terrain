#include "camera.h"
#include "core/frustum.h"

Camera::Camera()
{
	m_targetPosition = m_position = glm::vec3(0.0f, 1.0f, -3.0f);
	m_targetRotation = m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	m_fov = glm::radians(60.0f);
	m_aspect = 4.0f / 3.0f;
	m_nearPlane = 0.2f;
	m_farPlane = 2000.0f;

	m_target = glm::vec3(0.0f, 0.0f, 1.0f);
	m_speed = 2.0f;
	m_sensitivity = 0.8f;

	m_frustum = CreateRef<Frustum>();
	calculate_projection();
	calculate_view();
}

Ref<Camera> Camera::clone()
{
	std::shared_ptr<Camera> camera = CreateRef<Camera>();
	camera->m_position = m_position;
	camera->m_rotation = m_rotation;

	camera->m_targetPosition = m_targetPosition;
	camera->m_targetRotation = m_targetRotation;

	camera->m_projection = m_projection;
	camera->m_invProjection = m_invProjection;

	camera->m_fov = m_fov;
	camera->m_aspect = m_aspect;
	camera->m_nearPlane = m_nearPlane;
	camera->m_farPlane = m_farPlane;

	camera->m_target = m_target;
	camera->m_right = m_right;
	camera->m_up = m_up;
	camera->m_forward = m_forward;
	camera->m_speed = m_speed;
	camera->m_sensitivity = m_sensitivity;
	return camera;


}

void Camera::update(float dt)
{
#if 1
	m_position.x += (m_targetPosition.x - m_position.x) * 0.99f * dt * 2.0f;
	m_position.y += (m_targetPosition.y - m_position.y) * 0.99f * dt * 4.0f;
	m_position.z += (m_targetPosition.z - m_position.z) * 0.99f * dt * 2.0f;
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
	std::array<glm::vec3, 8> frustumPoints = {};
	glm::mat4 invCam = glm::inverse(m_projection * m_view);
	for (uint32_t i = 0; i < 8; i++) 
	{
		glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
		frustumPoints[i] = invCorner / invCorner.w;
	}

	m_frustum->set_points(frustumPoints);
}

std::array<glm::vec3, 8> Camera::get_frustum_point(float nearPlane, float farPlane)
{
	glm::vec3 nearPoint = m_position + nearPlane * m_forward;
	glm::vec3 farPoint = m_position + farPlane * m_forward;

	//Calculate point in near Plane
	float hH = tan(m_fov * 0.5f) * nearPlane;
	float hW = m_aspect * hH;

	std::array<glm::vec3, 8> frustumPoints;
	frustumPoints[0] = nearPoint - hW * m_right + hH * m_up; //ntl
	frustumPoints[1] = nearPoint + hW * m_right + hH * m_up; //ntr
	frustumPoints[2] = nearPoint + hW * m_right - hH * m_up; // nbr
	frustumPoints[3] = nearPoint - hW * m_right - hH * m_up; // nbl

	hH = tan(m_fov * 0.5f) * farPlane;
	hW = m_aspect * hH;
	frustumPoints[4] = farPoint - hW * m_right + hH * m_up; //ftl
	frustumPoints[5] = farPoint + hW * m_right + hH * m_up; //ftr
	frustumPoints[6] = farPoint + hW * m_right - hH * m_up; //fbr
	frustumPoints[7] = farPoint - hW * m_right - hH * m_up; //fbl
	return frustumPoints;
}

Ray Camera::generate_ray(const glm::vec2& mouse, const glm::vec2& windowSize)
{
	// clip space direction
	glm::vec4 d = glm::vec4((mouse.x / windowSize.x) * 2.0f - 1.0f, 1.0f - 2.0f * (mouse.y / windowSize.y), 0.0f, 0.0f);

	// camera space direction
	d = m_invProjection * d;
	d.z = -1.0f;
	d.w = 0.0f;

	// world space direction
	glm::vec3 worldSpace = glm::vec3(m_invView * d);
	return Ray{ m_position, glm::normalize(worldSpace) };
}

void Camera::calculate_projection()
{
	m_projection = glm::perspective(m_fov, m_aspect, m_nearPlane, m_farPlane);
	m_invProjection = glm::inverse(m_projection);
}

void Camera::calculate_view()
{
	glm::mat3 rotate = glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z);
	glm::vec3 up = glm::normalize(rotate * vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 forward = glm::normalize(rotate * vec3(0.0f, 0.0f, 1.0f));
	m_view = glm::lookAt(m_position, m_position + forward , up);
	m_invView = glm::inverse(m_view);

	m_right   = glm::vec3(m_view[0][0], m_view[1][0], m_view[2][0]);
	m_up      = glm::vec3(m_view[0][1], m_view[1][1], m_view[2][1]);
	m_forward = glm::vec3(m_view[0][2], m_view[1][2], m_view[2][2]);
}
