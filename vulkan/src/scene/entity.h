#pragma once

#include "core/base.h"
#include "core/math.h"
#include "mesh.h"

#include <string>
#include <memory>

struct Transform
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::fquat rotation = glm::fquat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 get_mat4()
	{
		rotation = glm::normalize(rotation);
		return glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}

	void set_rotation(const glm::vec3& axis, float angle)
	{
		rotation = glm::angleAxis(angle, axis);
	}
};
struct Rigidbody;

class Entity
{
public:
	Entity(std::string name, Ref<Transform> transform) : name(name), transform(transform), mesh(nullptr){}
	Entity(Ref<Transform> transform) :transform(transform), mesh(nullptr) {}

	Ref<Transform> transform;
	Ref<Mesh> mesh;
	Ref<Rigidbody> rigidBody;
	std::string name;
};

typedef std::vector<Entity*>::iterator EntityIterator;