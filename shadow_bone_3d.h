#pragma once

#include "kusudama_constraint.h"
#include "ik_node_3d.h"

#include "core/object/reference.h"

class KusudamaConstraint;
class ShadowBone3D {
	friend class EWBIKState;
	friend class IKNode3D;
	// BoneId for_bone;
	IKNode3D sim_local_ik_node;
	IKNode3D sim_constraint_ik_node;
	float cos_half_dampen = 0.0f;
	Vector<float> cos_half_returnful_dampened;
	Vector<float> half_returnful_dampened;
	bool springy = false;
	float pain = 0.0f;
	Ref<KusudamaConstraint> constraint;
	void set_constraint(Ref<KusudamaConstraint> p_constraint);
	Ref<KusudamaConstraint> get_constraint() const;
	void populate_return_dampening_iteration_array(int p_default_iterations, float p_default_dampening);
	void update_cos_dampening(int p_default_iterations, float p_default_dampening);
};