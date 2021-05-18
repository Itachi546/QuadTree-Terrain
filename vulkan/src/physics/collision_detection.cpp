#include "collision_detection.h"
#include "rigidbody.h"
#include "sphere_collider.h"
#include "plane_collider.h"
#include "manifold.h"

bool CollisionDetection::CheckIntersection(Ref<Rigidbody> ra, Ref<Rigidbody> rb, Ref<Manifold> manifold)
{
	Ref<Transform> tA = ra->transform;
	Ref<Collider> cA = ra->collider;

	Ref<Transform> tB = rb->transform;
	Ref<Collider> cB = rb->collider;

	ColliderType cAType = cA->get_collider_type();
	ColliderType cBType = cB->get_collider_type();

	if (cAType == ColliderType::Sphere && cBType == ColliderType::Plane)
	{
		return SpherePlaneIntersection(tA, std::static_pointer_cast<SphereCollider>(cA),
			                           tB, std::static_pointer_cast<PlaneCollider>(cB), manifold);
	}
	else if (cAType == ColliderType::Plane && cBType == ColliderType::Sphere)
	{
		manifold->swap_object();
		return SpherePlaneIntersection(tB, std::static_pointer_cast<SphereCollider>(cB),
			tA, std::static_pointer_cast<PlaneCollider>(cA), manifold);
	}
	else if (cAType == ColliderType::Sphere && cBType == ColliderType::Sphere)
	{
		return SphereSphereIntersection(tA, std::static_pointer_cast<SphereCollider>(cA),
			                            tB, std::static_pointer_cast<SphereCollider>(cB), manifold);
	}

	ASSERT_MSG(1, "Undefined Shape");
	return false;
}

bool CollisionDetection::SpherePlaneIntersection(Ref<Transform> tA, Ref<SphereCollider> cA, Ref<Transform> tB, Ref<PlaneCollider> cB, Ref<Manifold> manifold)
{
	// Equation of Plane
	// Ax + By + Cz = D
    // d = D - dot(p, n)	
	// Single sided plane
	glm::vec3 center = tA->position + cA->center;
	float radius = cA->radius * tA->scale.x;

	float distance = -cB->distance + glm::dot(cB->normal, center);
	if (std::abs(distance) <= radius)
	{
		ContactPoint contactPoint = {};
		contactPoint.normal = -cB->normal;
		contactPoint.penetration = radius - distance;
		ASSERT(contactPoint.penetration >= 0.0f);
		contactPoint.p = center - cB->normal * distance;
		manifold->add_contact(contactPoint);
		return true;
	}
	return false;
}


bool CollisionDetection::SphereSphereIntersection(Ref<Transform> tA, Ref<SphereCollider> cA, Ref<Transform> tB, Ref<SphereCollider> cB, Ref<Manifold> manifold)
{
	float radiiSum = cA->radius * tA->scale.x + cB->radius * tB->scale.x;
	glm::vec3 centerA = tA->position + cA->center;
	glm::vec3 centerB = tB->position + cB->center;
	glm::vec3 normal = (centerB - centerA);
	float distance = glm::length2(normal);

	if (distance < radiiSum * radiiSum)
	{
		distance = std::sqrtf(distance);
		normal = glm::normalize(normal);
		float depth = radiiSum - distance;
		ASSERT(depth >= 0.0f);

		ContactPoint contactPoint = {};
		contactPoint.normal = normal;
		contactPoint.penetration = depth;
		contactPoint.p = centerB - normal * cB->radius * tB->scale.x;

		manifold->add_contact(contactPoint);
		return true;
	}
	return false;
}