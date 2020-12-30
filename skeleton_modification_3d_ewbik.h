/*************************************************************************/
/*  skeleton_ik_constraints.h                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef MULTI_CONSTRAINT_H
#define MULTI_CONSTRAINT_H

#include "core/object/reference.h"
#include "core/os/memory.h"
#include "scene/resources/skeleton_modification_3d.h"

#include "bone_effector.h"
#include "direction_constraint.h"
#include "dmik_task.h"
#include "ewbik_transform.h"
#include "kusudama_constraint.h"
#include "qcp.h"
#include "twist_constraint.h"

class Skeleton3D;
class PhysicalBone3D;
class EWBIKBoneChainTarget;
class SkeletonModification3DEWBIK;
class EWBIKBoneChainTarget;
class EWBIKBoneEffector;
class Skeleton3D;
class EWBIKShadowSkeletonBone;
class SkeletonModificationStack3D;
class EWBIKSkeletonIKState;
class DirectionConstraint;
class TwistConstraint;
class KusudamaConstraint;

class IKNode3D : public Reference {
	GDCLASS(IKNode3D, Reference);

	IKBasis local;
	IKBasis global;
	Vector<Ref<IKNode3D>> children_nodes;
	Ref<IKNode3D> parent_node;
	bool dirty = false;
public:
	void mark_dirty() {
		dirty = true;
	}
	bool is_dirty() const {
		return dirty;
	}
	IKBasis get_local() const { return local; }
	IKBasis get_global() const { return global; }
	void set_local(IKBasis p_local) {
		local = p_local;
		mark_dirty();
	}
	void set_parent(Ref<IKNode3D> p_parent) {
		parent_node = p_parent;
		mark_dirty();
	}
	void update_global() {}
	void set_relative_to_parent(Ref<IKNode3D> par) {}
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
	void translate_to(Vector3 p_target) {
		update_global();
		if (parent_node.is_valid()) {
			local.translate_to(parent_node->get_global().get_local_of(p_target));
			mark_dirty();
		} else {
			local.translate_to(p_target);
			mark_dirty();
		}
	}
	Ray get_ray_x() {
		update_global();
		return get_global_ik_basis().get_x_ray();
	}
	Ray get_ray_y() {
		update_global();
		return get_global_ik_basis().get_y_ray();
	}
	Ray get_ray_z() {
		update_global();
		return get_global_ik_basis().get_z_ray();
	}

	void rotate_about_x(float angle) {
		update_global();
		Quat xRot = Quat(get_global_ik_basis().get_x_heading(), angle);
		rotate_by(xRot);
		mark_dirty();
	}

	void rotate_about_y(float angle) {
		update_global();
		Quat yRot = Quat(get_global_ik_basis().get_y_heading(), angle);
		rotate_by(yRot);
		mark_dirty();
	}

	void rotate_about_z(float angle) {
		update_global();
		Quat zRot = Quat(get_global_ik_basis().get_z_heading(), angle);
		rotate_by(zRot);
		mark_dirty();
	}
	// void set_rotation(Rot newRotation) {
	// 	this.rotation.set(newRotation);
	// 	this.refreshPrecomputed();
	// }
	void rotate_by(Quat addRotation) {
		update_global();
		if (parent_node.is_valid()) {
			Quat newRot = parent_node->get_global_ik_basis().get_local_of_rotation(addRotation);
			get_local_ik_basis().rotate_by(newRot);
		} else {
			get_local_ik_basis().rotate_by(addRotation);
		}
		mark_dirty();
	}
	IKBasis get_global_ik_basis() {
		update_global();
		return global;
	}
	IKBasis get_local_ik_basis() {
		return local;
	}
};

class SkeletonModification3DEWBIK : public SkeletonModification3D {
	GDCLASS(SkeletonModification3DEWBIK, SkeletonModification3D);

	Vector<Ref<EWBIKBoneEffector>> multi_effector;
	Ref<EWBIKSkeletonIKState> skeleton_ik_state;
	int32_t constraint_count = 0;
	int32_t effector_count = 0;
	Ref<EWBIKTask> task;
	String root_bone;
	int32_t default_stabilizing_pass_count = 4;
	Ref<QCP> qcp_convergence_check;
	inline static const Vector3 x_orientation = Vector3(1.0f, 0.0f, 0.0f);
	inline static const Vector3 y_orientation = Vector3(0.0f, 1.0f, 0.0f);
	inline static const Vector3 z_orientation = Vector3(0.0f, 0.0f, 1.0f);
	static const int32_t x_axis = 0;
	static const int32_t y_axis = 1;
	static const int32_t z_axis = 2;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

public:
	virtual void execute(float delta) override;
	virtual void setup_modification(SkeletonModificationStack3D *p_stack) override;
	static void iterated_improved_solver(Ref<QCP> p_qcp, int32_t p_root_bone, Ref<EWBIKShadowSkeletonBone> start_from, float dampening, int iterations, int p_stabilization_passes);
	static void grouped_recursive_chain_solver(Ref<EWBIKShadowSkeletonBone> p_start_from, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations);
	static void recursive_chain_solver(Ref<EWBIKShadowSkeletonBone> p_armature, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations);
	static void apply_bone_chains(float p_strength, Skeleton3D *p_skeleton, Ref<EWBIKShadowSkeletonBone> p_current_chain);
	void add_effector(String p_name, NodePath p_node = NodePath(), Transform p_transform = Transform(), real_t p_budget = 4.0f);
	void register_constraint(Skeleton3D *p_skeleton);
	Vector<Ref<EWBIKBoneEffector>> get_bone_effectors() const;
	int32_t find_effector(String p_name);
	void remove_effector(int32_t p_index);
	void set_effector_count(int32_t p_value);
	int32_t get_effector_count() const;
	Ref<EWBIKBoneEffector> get_effector(int32_t p_index) const;
	void set_effector(int32_t p_index, Ref<EWBIKBoneEffector> p_effector);
	String get_root_bone() const;
	void set_root_bone(String p_root_bone);
	SkeletonModification3DEWBIK();
	~SkeletonModification3DEWBIK();

	Ref<EWBIKSkeletonIKState> get_skeleton_ik_state() const { return skeleton_ik_state; }
	void set_skeleton_ik_data(Ref<EWBIKSkeletonIKState> val) { skeleton_ik_state = val; }

private:
	/**
     * The default maximum number of radians a bone is allowed to rotate per solver iteration.
     * The lower this value, the more natural the pose results. However, this will increase the number of iterations
     * the solver requires to converge.
     *
     * !!THIS IS AN EXPENSIVE OPERATION.
     * This updates the entire armature's cache of precomputed quadrance angles.
     * The cache makes things faster in general, but if you need to dynamically change the dampening during a call to IKSolver, use
     * the IKSolver(bone, dampening, iterations, stabilizationPasses) function, which clamps rotations on the fly.
     * @param damp
     */
	static void set_default_dampening(Ref<EWBIKShadowSkeletonBone> r_chain, float p_damp);
	static void update_armature_segments(Ref<EWBIKShadowSkeletonBone> r_chain);
	static void update_optimal_rotation_to_target_descendants(
			Skeleton3D *p_skeleton,
			Ref<EWBIKShadowSkeletonBone> p_chain_item,
			float p_dampening,
			bool p_is_translate,
			Vector<Vector3> p_localized_tip_headings,
			Vector<Vector3> p_localized_target_headings,
			Vector<real_t> p_weights,
			Ref<QCP> p_qcp_orientation_aligner,
			int p_iteration,
			float p_total_iterations);
	static void recursively_update_bone_segment_map_from(Ref<EWBIKShadowSkeletonBone> r_chain, Ref<EWBIKShadowSkeletonBone> p_start_from);
	static void QCPSolver(
			Skeleton3D *p_skeleton,
			Ref<EWBIKShadowSkeletonBone> p_chain,
			float p_dampening,
			bool p_inverse_weighting,
			int p_stabilization_passes,
			int p_iteration,
			float p_total_iterations);
	static bool build_chain(Ref<EWBIKTask> p_task);
	static void update_chain(Skeleton3D *p_sk, Ref<EWBIKShadowSkeletonBone> p_chain_item);
	static void solve_simple(Ref<EWBIKTask> p_task);

public:
	/**
     *
     * @param for_bone
     * @param dampening
     * @param translate set to true if you wish to allow translation in addition to rotation of the bone (should only be used for unpinned root bones)
     * @param stabilization_passes If you know that your armature isn't likely to succumb to instability in unsolvable configurations, leave this value set to 0.
     * If you value stability in extreme situations more than computational speed, then increase this value. A value of 1 will be completely stable, and just as fast
     * as a value of 0, however, it might result in small levels of robotic looking jerk. The higher the value, the less jerk there will be (but at potentially significant computation cost).
     */
	static void update_optimal_rotation_to_target_descendants(
			Skeleton3D *p_skeleton,
			Ref<EWBIKShadowSkeletonBone> r_chain,
			Ref<EWBIKShadowSkeletonBone> p_for_bone,
			float p_dampening,
			bool p_translate,
			int p_stabilization_passes,
			int p_iteration,
			int p_total_iterations);
	static float
	get_manual_msd(Vector<Vector3> &r_localized_effector_headings, Vector<Vector3> &r_localized_target_headings,
			const Vector<real_t> &p_weights);
	static void update_target_headings(Ref<EWBIKShadowSkeletonBone> r_chain, Vector<Vector3> &r_localized_target_headings,
			Vector<real_t> &p_weights, Transform p_bone_xform);
	static void update_effector_headings(Ref<EWBIKShadowSkeletonBone> r_chain, Vector<Vector3> &r_localized_effector_headings,
			Transform p_bone_xform);
	static Ref<EWBIKTask> create_simple_task(Skeleton3D *p_sk, String p_root_bone,
			float p_dampening = -1, int p_stabilizing_passes = -1,
			Ref<SkeletonModification3DEWBIK> p_constraints = NULL);
	static void solve(Ref<EWBIKTask> p_task, float blending_delta);
};
// Skeleton data structure
class EWBIKSkeletonIKState : public Resource {
	GDCLASS(EWBIKSkeletonIKState, Resource);
	friend class SkeletonModification3DEWBIK;
	Ref<SkeletonModification3DEWBIK> mod;
	Skeleton3D *skeleton = nullptr;

public:
	// It holds a bunch of references to bones thing
	// same index as the skeleton bone
	// ik info object.
	// ik data has:
	float get_stiffness(int32_t p_bone) const;
	void set_stiffness(int32_t p_bone, float p_stiffness_scalar);
	float get_height(int32_t p_bone) const;
	void set_height(int32_t p_bone, float p_height);
	Ref<KusudamaConstraint> get_constraint(int32_t p_bone) const;
	void set_constraint(int32_t p_bone, Ref<KusudamaConstraint> p_constraint);
	void init(Ref<SkeletonModification3DEWBIK> p_mod);
	~EWBIKSkeletonIKState();

protected:
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _get(const StringName &p_name, Variant &r_ret) const;
	bool _set(const StringName &p_name, const Variant &p_value);
};
#endif //MULTI_CONSTRAINT_H
