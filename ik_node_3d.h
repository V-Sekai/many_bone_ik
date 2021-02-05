#pragma once

#include "core/templates/vector.h"
#include "core/math/transform.h"
#include "core/math/basis.h"

#include "ewbik_transform.h"

class EWBIKState;
class IKNode3D {
	friend class EWBIKState;
	friend class ShadowBone3D;
	IKBasis pose_local;
	IKBasis pose_global;
	Vector<int32_t> child_bones;
	int32_t parent = -1;
	float height = 0.0f;
	float stiffness = 0.0f;
	bool dirty = true;

public:
	void set_height(float p_height);
	float get_height() const;
	void set_stiffness(float p_stiffness);
	float get_stiffness() const;
	IKBasis get_local() const;
	IKBasis get_global() const;
	Transform get_global_transform() const;
	void set_local(IKBasis p_local);
	void set_parent(int32_t p_parent);
	// void update_global() {}
	// void set_relative_to_parent(int32_t par) {}
	// Vector3 apply_global(Vector3 p_in) {
	// 	update_global();
	// }
	// Ray apply_global(Ray p_ray) {
	// 	update_global();
	// }
	// Vector3 get_global_of(Vector3 p_in) {
	// 	update_global();
	// 	// the other way around with transform xform
	// }
	// Ray get_local_of(Ray p_in) {
	// 	update_global();
	// 	// the other way around with transform xform
	// }
	// Create variations where the input parameter is directly changed
};