#pragma once
#include "scene/entity.h"

struct Transform;
class Collider;
struct Rigidbody
{
	Ref<Transform> transform;
	Ref<Collider> collider;
};
