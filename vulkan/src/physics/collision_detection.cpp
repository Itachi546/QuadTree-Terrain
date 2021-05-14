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

	ASSERT_MSG(1, "Undefined Shape");
	return false;
}

bool CollisionDetection::SpherePlaneIntersection(Ref<Transform> sphereTransform, Ref<SphereCollider> sphere, Ref<Transform> planeTransform, Ref<PlaneCollider> plane, Ref<Manifold> manifold)
{
	glm::vec3 center = sphereTransform->position;
	float radius = sphere->radius;

	float distance = plane->distance + glm::dot(plane->normal, center);
	if (std::abs(distance) <= radius)
	{
		ContactPoint contactPoint = {};
		contactPoint.normal = -plane->normal;
		contactPoint.penetration = radius - distance;
		manifold->add_contact(contactPoint);
		return true;
	}
	return false;
}
