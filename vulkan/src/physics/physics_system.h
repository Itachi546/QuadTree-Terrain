#pragma once

#include "core/base.h"
#include "rigidbody.h"
#include "plane_collider.h"
#include "sphere_collider.h"

#include <memory>
#include <vector>

class Scene;
class Manifold;

class PhysicsSystem
{
public:
	void init();
	void step(float dt);

	void add_rigid_body(Ref<Rigidbody> rb);
	void remove_rigid_body(Ref<Rigidbody> rb);

	void draw_manifolds();
	void destroy();

private:
	void check_collision();
	std::vector<Ref<Rigidbody>> m_rigidBodies;
	std::vector<Ref<Manifold>> m_manifolds;
};