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

#include "core/os/memory.h"
#include "core/object/reference.h"
#include "scene/resources/skeleton_modification_3d.h"

#include "bone_effector.h"
#include "dmik_task.h"
#include "kusudama_constraint.h"
#include "qcp.h"

class Skeleton3D;
class PhysicalBone3D;
class DMIKBoneChainTarget;
class SkeletonModification3DDMIK;
class KusudamaConstraint;
class DMIKBoneChainTarget;
class SkeletonModification3DDMIK;
class DMIKBoneEffector;
class Skeleton3D;
class DMIKShadowSkeletonBone;

// Skeleton data structure
class DMIKSkeletonIKData : public Resource {
public:
	// It holds a bunch of references to bones thing
	// same index as the skeleton bone
	// ik info object.
	// ik data has:
	// kusudama
	// all of the bone attributes
	// stiffness
	// float stiffnessScalar = 0.0f;
	// float get_stiffness() const {
	// 	return stiffnessScalar;
	// }

	// void set_stiffness(float p_stiffness_scalar) {
	// 	stiffnessScalar = p_stiffness_scalar;
	// }
	// height
	// references to children and parent
	// properties of bones. set and gets
};

class SkeletonModification3DDMIK : public SkeletonModification3D {
	GDCLASS(SkeletonModification3DDMIK, SkeletonModification3D);

	Vector<Ref<DMIKBoneEffector>> multi_effector;
	Vector<Ref<KusudamaConstraint>> multi_constraint;
	Ref<DMIKSkeletonIKData> skeleton_data;
	int32_t constraint_count = 0;
	int32_t effector_count = 0;
	Ref<DMIKTask> task;
	String root_bone;
	int32_t default_stabilizing_pass_count = 4;
	Ref<QCP> qcp_convergence_check;

private:
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
	static void iterated_improved_solver(Ref<QCP> p_qcp, int32_t p_root_bone, Ref<DMIKShadowSkeletonBone> start_from, float dampening, int iterations, int p_stabilization_passes);
	static void grouped_recursive_chain_solver(Ref<DMIKShadowSkeletonBone> p_start_from, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations);
	static void recursive_chain_solver(Ref<DMIKShadowSkeletonBone> p_armature, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations);
	static void apply_bone_chains(float p_strength, Skeleton3D *p_skeleton, Ref<DMIKShadowSkeletonBone> p_current_chain);
	void add_effector(String p_name, NodePath p_node = NodePath(), Transform p_transform = Transform(), real_t p_budget = 4.0f);
	void register_constraint(Skeleton3D *p_skeleton);
	void set_constraint_count(int32_t p_value);
	int32_t get_constraint_count() const;
	Vector<Ref<DMIKBoneEffector>> get_bone_effectors() const;
	int32_t find_effector(String p_name);
	void remove_effector(int32_t p_index);
	int32_t find_constraint(String p_name);
	void set_effector_count(int32_t p_value);
	int32_t get_effector_count() const;
	Ref<DMIKBoneEffector> get_effector(int32_t p_index) const;
	void set_effector(int32_t p_index, Ref<DMIKBoneEffector> p_effector);
	void set_constraint(int32_t p_index, Ref<KusudamaConstraint> p_constraint);
	Ref<KusudamaConstraint> get_constraint(int32_t p_index) const;
	String get_root_bone() const;
	void set_root_bone(String p_root_bone);
	SkeletonModification3DDMIK();
	~SkeletonModification3DDMIK();

	Ref<DMIKSkeletonIKData> get_skeleton_ik_data() const { return skeleton_data; }
	void set_skeleton_ik_data(Ref<DMIKSkeletonIKData> val) { skeleton_data = val; }

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
	static void set_default_dampening(Ref<DMIKShadowSkeletonBone> r_chain, float p_damp);
	static void update_armature_segments(Ref<DMIKShadowSkeletonBone> r_chain);
	static void update_optimal_rotation_to_target_descendants(
			Skeleton3D *p_skeleton,
			Ref<DMIKShadowSkeletonBone> p_chain_item,
			float p_dampening,
			bool p_is_translate,
			Vector<Vector3> p_localized_tip_headings,
			Vector<Vector3> p_localized_target_headings,
			Vector<real_t> p_weights,
			Ref<QCP> p_qcp_orientation_aligner,
			int p_iteration,
			float p_total_iterations);
	static void recursively_update_bone_segment_map_from(Ref<DMIKShadowSkeletonBone> r_chain, Ref<DMIKShadowSkeletonBone> p_start_from);
	static void QCPSolver(
			Skeleton3D *p_skeleton,
			Ref<DMIKShadowSkeletonBone> p_chain,
			float p_dampening,
			bool p_inverse_weighting,
			int p_stabilization_passes,
			int p_iteration,
			float p_total_iterations);
	static bool build_chain(Ref<DMIKTask> p_task);
	static void update_chain(Skeleton3D *p_sk, Ref<DMIKShadowSkeletonBone> p_chain_item);
	static void solve_simple(Ref<DMIKTask> p_task);

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
			Ref<DMIKShadowSkeletonBone> r_chain,
			Ref<DMIKShadowSkeletonBone> p_for_bone,
			float p_dampening,
			bool p_translate,
			int p_stabilization_passes,
			int p_iteration,
			int p_total_iterations);
	static float
	get_manual_msd(Vector<Vector3> &r_localized_effector_headings, Vector<Vector3> &r_localized_target_headings,
			const Vector<real_t> &p_weights);
	static void update_target_headings(Ref<DMIKShadowSkeletonBone> r_chain, Vector<Vector3> &r_localized_target_headings,
			Vector<real_t> &p_weights, Transform p_bone_xform);
	static void update_effector_headings(Ref<DMIKShadowSkeletonBone> r_chain, Vector<Vector3> &r_localized_effector_headings,
			Transform p_bone_xform);
	static Ref<DMIKTask> create_simple_task(Skeleton3D *p_sk, String p_root_bone,
			float p_dampening = -1, int p_stabilizing_passes = -1,
			Ref<SkeletonModification3DDMIK> p_constraints = NULL);
	static void solve(Ref<DMIKTask> p_task, float blending_delta);
};

#endif //MULTI_CONSTRAINT_H
