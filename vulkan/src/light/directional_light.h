#pragma once

#include "light.h"

class DirectionalLight : public Light
{
	friend class Scene;
public:
	DirectionalLight(const glm::vec3& direction) : m_direction(direction), Light(LightType::Directional){}
	void set_direction(const glm::vec3& direction) { m_direction = direction; }
	glm::vec3 get_direction() { return m_direction; }

private:
	glm::vec3 m_direction;
};