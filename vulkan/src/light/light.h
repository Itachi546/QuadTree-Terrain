#pragma once

#include "core/math.h"

enum class LightType
{
	Directional = 0
};

class Light
{
public:
	Light(LightType type) : m_lightType(type), m_intensity(1.0f), m_lightColor(1.0f), m_castShadow(false){}

	void set_intensity(float intensity) { m_intensity = intensity; }
	float get_intensity() { return m_intensity; }

	void set_light_color(const glm::vec3& color) { m_lightColor = color; }
	glm::vec3 get_light_color() { return m_lightColor; }

	bool cast_shadow() { return m_castShadow; }
	void set_cast_shadow(bool castShadow) { m_castShadow = castShadow; }

protected:
	LightType m_lightType;
	bool m_castShadow;
	float m_intensity;
	glm::vec3 m_lightColor;
};