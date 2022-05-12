#pragma once

#include "core/math/vector3.h"
#include "core/io/resource.h"

class IKLimitCone3D : public Resource {
	GDCLASS(IKLimitCone3D, Resource);
	Vector3 direction;
	float radius;
	Ref<IKLimitCone3D> nextSibling;
	float tangentConeRadius;
	Vector3 leftTangentConeDirection;
	Vector3 rightTangentConeDirection;

public:
	IKLimitCone3D() {}
	~IKLimitCone3D() {}
	Vector3 get_direction() const { return Vector3(); }
	void set_direction(Vector3 p_direction) { return; }
	float get_radius() const;
	void set_radius(float p_radius) { return; }
	Ref<IKLimitCone3D> get_nextSibling() const { return Ref<IKLimitCone3D>(); }
	void set_nextSibling(Ref<IKLimitCone3D> p_nextSibling) { return; }
	float get_tangentConeRadius() const { return 0.0f; }
	void set_tangentConeRadius(float p_tangentConeRadius) { return; }
	Vector3 get_leftTangentConeDirection() const { return Vector3(); }
	void set_leftTangentConeDirection(Vector3 leftTangentConeDirection);
	Vector3 get_rightTangentConeDirection() const { return Vector3(); }
	void set_rightTangentConeDirection(Vector3 p_rightTangentConeDirection);
};