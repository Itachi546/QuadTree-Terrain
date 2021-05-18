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
	static bool SpherePlaneIntersection(Ref<Transform> tA, Ref<SphereCollider> cA, Ref<Transform> tB, Ref<PlaneCollider> cB, Ref<Manifold> manifold);
	static bool SphereSphereIntersection(Ref<Transform> tA, Ref<SphereCollider> cA, Ref<Transform> tB, Ref<SphereCollider> cB, Ref<Manifold> manifold);
};