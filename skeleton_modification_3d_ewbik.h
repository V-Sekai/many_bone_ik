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
class EWBIKSegmentedSkeleton3D;
class SkeletonModificationStack3D;
class EWBIKState;
class DirectionConstraint;
class TwistConstraint;

class SkeletonModification3DEWBIK : public SkeletonModification3D {
	GDCLASS(SkeletonModification3DEWBIK, SkeletonModification3D);

	Vector<Ref<EWBIKBoneEffector>> multi_effector;
	Ref<EWBIKState> skeleton_ik_state;
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
	static void iterated_improved_solver(Ref<QCP> p_qcp, int32_t p_root_bone, Ref<EWBIKSegmentedSkeleton3D> start_from, float dampening, int iterations, int p_stabilization_passes);
	static void grouped_recursive_chain_solver(Ref<EWBIKSegmentedSkeleton3D> p_start_from, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations);
	static void recursive_chain_solver(Ref<EWBIKSegmentedSkeleton3D> p_armature, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations);
	static void apply_bone_chains(float p_strength, Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_current_chain);
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

	Ref<EWBIKState> get_state() const { return skeleton_ik_state; }
	void set_state(Ref<EWBIKState> val) { skeleton_ik_state = val; }

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
	static void set_default_dampening(Ref<EWBIKSegmentedSkeleton3D> r_chain, float p_damp);
	static void update_armature_segments(Ref<EWBIKSegmentedSkeleton3D> r_chain);
	static void update_optimal_rotation_to_target_descendants(
			Skeleton3D *p_skeleton,
			Ref<EWBIKSegmentedSkeleton3D> p_chain_item,
			float p_dampening,
			bool p_is_translate,
			Vector<Vector3> p_localized_tip_headings,
			Vector<Vector3> p_localized_target_headings,
			Vector<real_t> p_weights,
			Ref<QCP> p_qcp_orientation_aligner,
			int p_iteration,
			float p_total_iterations);
	static void recursively_update_bone_segment_map_from(Ref<EWBIKSegmentedSkeleton3D> r_chain, Ref<EWBIKSegmentedSkeleton3D> p_start_from);
	static void QCPSolver(
			Skeleton3D *p_skeleton,
			Ref<EWBIKSegmentedSkeleton3D> p_chain,
			float p_dampening,
			bool p_inverse_weighting,
			int p_stabilization_passes,
			int p_iteration,
			float p_total_iterations);
	static bool build_chain(Ref<EWBIKTask> p_task);
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
			Ref<EWBIKSegmentedSkeleton3D> r_chain,
			Ref<EWBIKSegmentedSkeleton3D> p_for_bone,
			float p_dampening,
			bool p_translate,
			int p_stabilization_passes,
			int p_iteration,
			int p_total_iterations);
	static float
	get_manual_msd(Vector<Vector3> &r_localized_effector_headings, Vector<Vector3> &r_localized_target_headings,
			const Vector<real_t> &p_weights);
	static void update_target_headings(Ref<EWBIKSegmentedSkeleton3D> r_chain, Vector<Vector3> &r_localized_target_headings,
			Vector<real_t> &p_weights, Transform p_bone_xform);
	static void update_effector_headings(Ref<EWBIKSegmentedSkeleton3D> r_chain, Vector<Vector3> &r_localized_effector_headings,
			Transform p_bone_xform);
	static Ref<EWBIKTask> create_simple_task(Skeleton3D *p_sk, String p_root_bone,
			float p_dampening = -1, int p_stabilizing_passes = -1,
			Ref<SkeletonModification3DEWBIK> p_constraints = NULL);
	static void solve(Ref<EWBIKTask> p_task, float blending_delta);
};
// Skeleton data structure
class EWBIKState : public Resource {
	GDCLASS(EWBIKState, Resource);

	class IKNode3D {
		friend class EWBIKState;
		IKBasis pose_local;
		IKBasis pose_global;
		Vector<int32_t> child_bones;
		int32_t parent = -1;
		float height = 0.0f;
		float stiffness = 0.0f;
		bool dirty = true;

	public:
		void set_height(float p_height) {
			height = p_height;
		}
		float get_height() const {
			return height;
		}
		void set_stiffness(float p_stiffness) {
			stiffness = p_stiffness;
		}
		float get_stiffness() const {
			return stiffness;
		}
		IKBasis get_local() const { return pose_local; }
		IKBasis get_global() const { return pose_global; }
		void set_local(IKBasis p_local) {
			pose_local = p_local;
		}
		void set_parent(int32_t p_parent) {
			parent = p_parent;
		}
		void update_global() {}
		void set_relative_to_parent(int32_t par) {}
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
		IKBasis get_global_ik_basis() {
			update_global();
			return pose_global;
		}
		IKBasis get_local_ik_basis() {
			return pose_local;
		}
	};
	friend class SkeletonModification3DEWBIK;
	class ShadowBone3D {
		friend class EWBIKState;
		BoneId for_bone;
		IKNode3D sim_local_ik_node;
		IKNode3D sim_constraint_ik_node;
		float cos_half_dampen = 0.0f;
		Vector<float> cos_half_returnful_dampened;
		Vector<float> half_returnful_dampened;
		bool springy = false;
		Ref<KusudamaConstraint> constraint;
		void set_constraint(Ref<KusudamaConstraint> p_constraint);
		Ref<KusudamaConstraint> get_constraint() const;
		void populate_return_dampening_iteration_array(int p_default_iterations, float p_default_dampening);
		void update_cos_dampening(int p_default_iterations, float p_default_dampening);
	};
	Ref<SkeletonModification3DEWBIK> mod;
	Skeleton3D *skeleton = nullptr;
	int bone_count = 0;

	Vector<ShadowBone3D> bones;
	Vector<int32_t> parentless_bones;

public:
	float get_cos_half_dampen(int32_t p_bone) const;
	void set_cos_half_dampen(int32_t p_bone, float p_cos_half_dampen);
	// Vector<float> get_cos_half_returnful_dampened(int32_t p_bone) const;
	// Vector<float> get_half_returnful_dampened(int32_t p_bone) const;
	// void set_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened);
	// void set_cos_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened);
	Transform global_constraint_pose_to_local_pose(int p_bone_idx, Transform p_global_pose);
	Transform global_shadow_pose_to_local_pose(int p_bone_idx, Transform p_global_pose);
	void force_update_bone_children_transforms(int p_bone_idx);
	void update_skeleton();
	void mark_dirty(int32_t p_bone);
	bool is_dirty(int32_t p_bone) const;
	void _update_process_order();
	void rotate_by(int32_t p_bone, Quat addRotation);
	void translate_to(int32_t p_bone, Vector3 p_target);
	Ray get_ray_x(int32_t p_bone);
	Ray get_ray_y(int32_t p_bone);
	Ray get_ray_z(int32_t p_bone);
	void rotate_about_x(int32_t p_bone, float angle);
	void rotate_about_y(int32_t p_bone, float angle);
	void rotate_about_z(int32_t p_bone, float angle);
	// void set_rotation(Rot newRotation) {
	// 	this.rotation.set(newRotation);
	// 	this.refreshPrecomputed();
	// }
	int32_t get_parent(int32_t p_bone) const;
	void set_parent(int32_t p_bone, int32_t p_parent);
	void set_shadow_bone_pose_local(int p_bone, const Transform &value);
	void align_shadow_bone_globals_to(int p_bone, Transform p_target);
	void align_shadow_constraint_globals_to(int p_bone, Transform p_target);
	Transform get_shadow_pose_local(int p_bone) const;
	Transform get_shadow_pose_global(int p_bone) const;
	Transform get_shadow_constraint_pose_global(int p_bone) const;
	Transform get_shadow_constraint_pose_local(int p_bone) const;
	void set_shadow_constraint_pose_local(int p_bone, const Transform &value);
	void set_shadow_bone_dirty(int p_bone);
	float get_stiffness(int32_t p_bone) const;
	void set_stiffness(int32_t p_bone, float p_stiffness_scalar);
	float get_height(int32_t p_bone) const;
	void set_height(int32_t p_bone, float p_height);

	Ref<KusudamaConstraint> get_constraint(int32_t p_bone) const;
	void set_constraint(int32_t p_bone, Ref<KusudamaConstraint> p_constraint);
	void init(Ref<SkeletonModification3DEWBIK> p_mod);
	int32_t get_bone_count() const { return bone_count; }
	void set_bone_count(int32_t p_bone_count);

protected:
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _get(const StringName &p_name, Variant &r_ret) const;
	bool _set(const StringName &p_name, const Variant &p_value);
};
#endif //MULTI_CONSTRAINT_H
