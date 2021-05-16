#pragma once
#include "core/base.h"
#include "manifold.h"

struct RigidBody;
class  PlaneCollider;
class  SphereCollider;
struct Transform;

class CollisionDetection
{
public:
	static bool CheckIntersection(Ref<Rigidbody> ra, Ref<Rigidbody> rb, Ref<Manifold> manifold);

private:
	static bool SpherePlaneIntersection(Ref<Transform> sphereTransform, Ref<SphereCollider> sphere, Ref<Transform> planeTransform, Ref<PlaneCollider> plane, Ref<Manifold> manifold);
};