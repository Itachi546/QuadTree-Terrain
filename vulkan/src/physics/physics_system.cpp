#include "physics_system.h"
#include "rigidbody.h"
#include "collision_detection.h"
#include "manifold.h"

#include "debug/debug_draw.h"

void PhysicsSystem::init()
{
}

void PhysicsSystem::step(float dt)
{
	m_manifolds.clear();
	check_collision();
}

void PhysicsSystem::add_rigid_body(Ref<Rigidbody> rb)
{
	m_rigidBodies.push_back(rb);
}

void PhysicsSystem::remove_rigid_body(Ref<Rigidbody> rb)
{
	auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rb);
	if (it != m_rigidBodies.end())
		m_rigidBodies.erase(it);
}

void PhysicsSystem::draw_manifolds()
{
	for (const auto& manifold : m_manifolds)
	{
		for (const auto& contact : manifold->contacts)
		{
			DebugDraw::draw_points(contact.p, glm::vec3(1.0f));
			DebugDraw::draw_line(contact.p, contact.p + contact.normal * contact.penetration, glm::vec3(1.0f, 0.0f, 0.0f), 2);
		}
	}
}

void PhysicsSystem::destroy()
{
	m_rigidBodies.clear();
}

void PhysicsSystem::check_collision()
{
	auto begin = m_rigidBodies.begin();
	auto end = m_rigidBodies.end();
	for (auto i = begin; i != end; ++i)
	{
		Ref<Rigidbody> a = *i;
		if (a->collider == nullptr) continue;
		for (auto j = i + 1; j != end; ++j)
		{
			// @TODO remove the creation of manifold
			Ref<Rigidbody> b = *j;
			Ref<Manifold> manifold = CreateRef<Manifold>(a, b);
			if (b->collider == nullptr) continue;
			if (CollisionDetection::CheckIntersection(a, b, manifold))
				m_manifolds.push_back(manifold);
		}
	}
}
