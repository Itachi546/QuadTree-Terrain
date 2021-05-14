#pragma once

enum class ColliderType
{
	Box,
	Sphere,
	Plane
};

class Collider
{
public:
	Collider(ColliderType type) : m_colliderType(type){}
	ColliderType get_collider_type() { return m_colliderType; }
	virtual ~Collider(){}
private:
	ColliderType m_colliderType;
};