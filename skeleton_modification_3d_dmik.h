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

#include "bone_effector.h"
#include "core/os/memory.h"
#include "core/reference.h"
#include "ik_axes.h"
#include "kusudama_constraint.h"
#include "qcp.h"
#include "scene/resources/skeleton_modification_3d.h"

class Skeleton3D;
class KusudamaConstraint;

class BoneChain;
class PhysicalBone3D;
class BoneChainItem : public Reference {
	GDCLASS(BoneChainItem, Reference);

public:
	Ref<BoneChain> parent_armature = nullptr;
	Vector<Ref<BoneChainItem>> children;
	Ref<BoneChainItem> parent_item = nullptr;
	// Bone info
	int bone = -1;
	PhysicalBone3D *pb = nullptr;
	bool springy = false;
	float cos_half_dampen = 0.0f;
	Vector<real_t> cos_half_returnful_dampened;
	Vector<real_t> half_returnful_dampened;
	bool ik_orientation_lock = false;
	float stiffness_scalar = 0.0f;
	float bone_height = 0.0f;
	float length = 0.0f;

	Transform axes;

	Ref<KusudamaConstraint> constraint = nullptr;

	BoneChainItem() {}

	float get_bone_height() const;

	void set_bone_height(const float p_bone_height);

	Ref<BoneChainItem> find_child(const int p_bone_id);

	Ref<BoneChainItem> add_child(const int p_bone_id);

	void set_stiffness(float p_stiffness);

	float get_stiffness() const;

	void update_cos_dampening();

	void
	set_axes_to_returned(IKAxes p_global, IKAxes p_to_set, IKAxes p_limiting_axes, float p_cos_half_angle_dampen,
			float p_angle_dampen);

	void set_axes_to_be_snapped(IKAxes p_to_set, IKAxes p_limiting_axes, float p_cos_half_angle_dampen);

	void populate_return_dampening_iteration_array(Ref<KusudamaConstraint> k);
	void rootwardly_update_falloff_cache_from(Ref<BoneChainItem> p_current);
};

class BoneChainTarget;
class SkeletonModification3DDMIK;
class BoneChain : public Reference {
	GDCLASS(BoneChain, Reference);

public:
	Ref<BoneChainItem> chain_root = memnew(BoneChainItem);
	Ref<BoneChainItem> middle_chain_item = nullptr;
	Vector<Ref<BoneChainTarget>> targets;
	Vector3 magnet_position;
	Vector<Vector3> localized_target_headings;
	Vector<Vector3> localized_effector_headings;
	Vector<real_t> weights;
	Ref<SkeletonModification3DDMIK> constraints = nullptr;
	float dampening = Math::deg2rad(5.0f);
	Map<int, Ref<BoneChainItem>> bone_segment_map;
	int ik_iterations = 15;

	int get_default_iterations() const;
	void recursively_create_penalty_array(Ref<BoneChain> from, Vector<Vector<real_t>> &r_weight_array, Vector<Ref<BoneChainItem>> pin_sequence, float current_falloff);
	void create_headings_arrays();
};

class BoneEffectorTransform : public Reference {
	GDCLASS(BoneEffectorTransform, Reference);

public:
	int effector_bone = -1;
	Transform goal_transform;
};
class BoneChainTarget : public Reference {
	GDCLASS(BoneChainTarget, Reference);

public:
	Ref<BoneChainItem> chain_item = nullptr;
	Ref<BoneEffectorTransform> end_effector = nullptr;

	BoneChainTarget() :
			chain_item(NULL),
			end_effector(NULL) {}

	BoneChainTarget(Ref<BoneChainItem> p_chain_item, const Ref<BoneEffectorTransform> p_end_effector) :
			chain_item(p_chain_item),
			end_effector(p_end_effector) {}

	BoneChainTarget(const Ref<BoneChainTarget> p_other_ct) :
			chain_item(p_other_ct->chain_item),
			end_effector(p_other_ct->end_effector) {}

	BoneChainTarget(Ref<BoneChainItem> p_chain_item, const Ref<BoneEffectorTransform> p_end_effector, bool p_enabled) {
		enabled = p_enabled;
		set_target_priorities(x_priority, y_priority, z_priority);
	}

protected:
	bool enabled;
	BoneChainTarget *parent_target = nullptr;
	Vector<BoneChainTarget *> child_targets;
	float target_weight = 1;
	uint8_t mode_code = 7;
	int sub_target_count = 4;
	float x_priority = 1.0f, y_priority = 1.0f, z_priority = 1.0f;
	float depthFalloff = 0.0f;

public:
	static const short XDir = 1, YDir = 2, ZDir = 4;

	bool is_enabled() const;

	void toggle();

	void enable();

	void disable();

	/**
     * Targets can be ultimate targets, or intermediary targets.
     * By default, each target is treated as an ultimate target, meaning
     * any bones which are ancestors to that target's end-effector
     * are not aware of any target wich are target of bones descending from that end effector.
     *
     * Changing this value makes ancestor bones aware, and also determines how much less
     * they care with each level down.
     *
     * Presuming all descendants of this target have a falloff of 1, then:
     * A target falloff of 0 on this target means only this target is reported to ancestors.
     * A target falloff of 1 on this target means ancestors care about all descendant target equally (after accounting for their pinWeight),
     * regardless of how many levels down they are.
     * A target falloff of 0.5 means each descendant target is cared about half as much as its ancestor.
     *
     * With each level, the target falloff of a descendant is taken account for each level.
     *  Meaning, if this target has a falloff of 1, and its descendent has a falloff of 0.5
     *  then this target will be reported with full weight,
     *  it descendant will be reported with full weight,
     *  the descendant of that target will be reported with half weight.
     *  the desecendant of that one's descendant will be reported with quarter weight.
     *
     * @param p_depth
     */
	void set_depth_falloff(float p_depth);

	float get_depth_falloff() const;

	/**
     * Sets  the priority of the orientation bases which effectors reaching for this target will and won't align with.
     * If all are set to 0, then the target is treated as a simple position target.
     * It's usually better to set at least on of these three values to 0, as giving a nonzero value to all three is most often redundant.
     *
     *  This values this function sets are only considered by the orientation aware solver.
     *
     * @param position
     * @param p_x_priority set to a positive value (recommended between 0 and 1) if you want the bone's x basis to point in the same direction as this target's x basis (by this library's convention the x basis corresponds to a limb's twist)
     * @param p_y_priority set to a positive value (recommended between 0 and 1)  if you want the bone's y basis to point in the same direction as this target's y basis (by this library's convention the y basis corresponds to a limb's direction)
     * @param p_z_priority set to a positive value (recommended between 0 and 1)  if you want the bone's z basis to point in the same direction as this target's z basis (by this library's convention the z basis corresponds to a limb's twist)
     */
	void set_target_priorities(float p_x_priority, float p_y_priority, float p_z_priority);

	/**
     * @return the number of bases an effector to this target will attempt to align on.
     */
	int get_subtarget_count();

	uint8_t get_mode_code() const;

	/**
     * @return the priority of this target's x axis;
     */
	float get_x_priority() const;

	/**
     * @return the priority of this target's y axis;
     */
	float get_y_priority() const;

	/**
     * @return the priority of this target's z axis;
     */
	float get_z_priority() const;

	IKAxes get_axes() const;

	/**
     * translates and rotates the target to match the position
     * and orientation of the input Axes. The orientation
     * is only relevant for orientation aware solvers.
     * @param inAxes
     */
	void align_to_axes(IKAxes inAxes);

	/**
     * translates the pin to the location specified in local coordinates
     * (relative to any other Axes objects the pin may be parented to)
     * @param location
     */
	void translate(Vector3 location);

	/**
     * @return the target location in global coordinates
     */
	Vector3 get_location();

	Ref<BoneChainItem> for_bone();

	/**
     * called when this target is being removed entirely from the Armature. (as opposed to just being disabled)
     */
	void removal_notification();

	void set_parent_target(BoneChainTarget *parent);

	void remove_child_target(BoneChainTarget *child);

	void add_child_target(BoneChainTarget *new_child);

	BoneChainTarget *get_parent_target();

	bool is_ancestor_of(BoneChainTarget *potential_descendent);

	float get_target_weight();
};

class Skeleton3D;
class DMIKTask : public Reference {
	GDCLASS(DMIKTask, Reference);

protected:
	static void _bind_methods() {}

public:
	Skeleton3D *skeleton = nullptr;

	Ref<BoneChain> chain = memnew(BoneChain);

	// Settings
	float min_distance = 0.01f;
	int iterations = 4;
	int max_iterations = 1.0f;
	// dampening dampening angle in radians.
	// Set this to -1 if you want to use the armature's default.
	float dampening = 0.05f;
	// stabilizing_passes number of stabilization passes to run.
	// Set this to -1 if you want to use the armature's default.
	int stabilizing_passes = -1;

	// Bone data
	int root_bone = -1;
	Vector<Ref<BoneEffectorTransform>> end_effectors;
};

class BoneChainTwo : public Resource {
	GDCLASS(BoneChainTwo, Resource);

private:
	Vector<Ref<BoneEffector>> multi_effector;
	// a list of bonechains which are children of this chain
	Vector<Ref<BoneChainTwo>> child_chains;
	// a list of Bones contained in this chain.
	Vector<BoneId> bones;
	BoneId base_bone = -1;
	BoneId tip_bone = -1;
	Skeleton3D *skeleton = nullptr;
	Ref<BoneChainTwo> parent_chain; //contains the parentChain of this Bonechain, if any.

	//will be set to true if this chain or any of its descendants have an effector.
	//a postprocessing step will remove any chains which are not active
	bool is_active = false;

	//will be set to true if the tip of this chain is an effector.
	bool has_effector = false;

	void build_chain(BoneId p_start_from) {
		BoneId current_bone = p_start_from;
		while (true) {
			bones.push_back(current_bone);
			tip_bone = current_bone;
			bool is_in_multi_effectors = false;
			Ref<BoneEffector> effector;
			for (int32_t i = 0; i < multi_effector.size(); i++) {
				effector = multi_effector[i];
				if (effector.is_null()) {
					continue;
				}
				String bone_name = skeleton->get_bone_name(current_bone);
				if (effector->get_name() == bone_name) {
					is_in_multi_effectors = true;
					break;
				}
			}
			Vector<int32_t> current_bone_children = get_bone_children(skeleton, current_bone);
			if (current_bone_children.size() != 1 || has_effector) {
				create_child_chains(current_bone);
				if (is_in_multi_effectors) {
					has_effector = true;
					set_active();
				}
				break;
			}
			current_bone = current_bone_children[0]; //by definition, there is only one child to this bone if we reach this line.
		}
	}
	void create_child_chains(BoneId p_from_bone) {
		Vector<int32_t> children = get_bone_children(skeleton, p_from_bone);
		for (int i = 0; i < children.size(); i++) {
			Ref<BoneChainTwo> bone_chain;
			bone_chain.instance();
			BoneId child = children[i];
			bone_chain->init(skeleton, multi_effector, this, child);
			child_chains.push_back(bone_chain);
		}
	}

	void remove_inactive_children() {
		for (int i = 0; i < child_chains.size(); i++) {
			if (!child_chains[i]->is_active) {
				child_chains.remove(i);
			}
		}
	}

	void merge_with_child_if_appropriate() {
		if (child_chains.size() == 1 && has_effector != true) {
			Ref<BoneChainTwo> child = child_chains[0];
			tip_bone = child->tip_bone;
			has_effector = child->has_effector;
			Vector<BoneId> child_bones = child->bones;
			for (int i = 0; i < child_bones.size(); i++) {
				bones.push_back(child_bones[i]);
			}
			child_chains.append_array(child_chains);
		}
	}

public:
	// TODO REMOVE
	Vector<int32_t> get_bones() {
		return bones;
	}
	// TODO REMOVE
	Vector<Ref<BoneChainTwo>> get_child_chains() {
		return child_chains;
	}
	static Vector<int32_t> get_bone_children(Skeleton3D *p_skeleton, int32_t p_bone) {
		Map<int32_t, Vector<int32_t>> parent_child_bones;
		for (int32_t bone_i = 0; bone_i < p_skeleton->get_bone_count(); bone_i++) {
			int32_t parent = p_skeleton->get_bone_parent(bone_i);
			Vector<int32_t> children;
			if (parent_child_bones.has(parent)) {
				children = parent_child_bones[parent];
			}
			children.push_back(bone_i);
			parent_child_bones[parent] = children;
		}
		return parent_child_bones[p_bone];
	}
	void init(Skeleton3D *p_skeleton, Vector<Ref<BoneEffector>> &p_multi_effector, Ref<BoneChainTwo> p_parent_chain, BoneId p_base_bone) {
		ERR_FAIL_COND(this == parent_chain.ptr());
		multi_effector = p_multi_effector;
		parent_chain = p_parent_chain;
		base_bone = p_base_bone;
		skeleton = p_skeleton;
		build_chain(p_base_bone);
	}

	/**sets this bonechain and all of its ancestors to active */

	void set_active() {
		is_active = true;
		if (parent_chain.is_valid()) {
			parent_chain->set_active();
		}
	}

	/**
     * removes any child chains which are not active.
     *
     * if this chain's tip isn't effectored, and it only has one active childchain,
     * it also merges this chain with the child chain.
     *
     * This is done recursively.
     * @return
     */
	void filter_and_merge_child_chains() {
		remove_inactive_children();
		merge_with_child_if_appropriate();
		for (int i = 0; i < child_chains.size(); i++) {
			child_chains.write[i]->filter_and_merge_child_chains();
		}
	}
};

class SkeletonModification3DDMIK : public SkeletonModification3D {
	GDCLASS(SkeletonModification3DDMIK, SkeletonModification3D);

	Vector<Ref<BoneEffector>> multi_effector;
	Vector<Ref<KusudamaConstraint>> multi_constraint;
	int32_t constraint_count = 0;
	int32_t effector_count = 0;
	Ref<DMIKTask> task;
	String root_bone;
	Vector<Vector<String>> bone_chains;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();

public:
	virtual void execute(float delta);
	virtual void setup_modification(SkeletonModificationStack3D *p_stack);
	void print_bone_chains(Ref<BoneChainTwo> p_bone_chain) {
		int32_t count = 0;
		List<Ref<BoneChainTwo>> queue;
		queue.push_back(p_bone_chain);
		while (queue.size()) {
			Ref<BoneChainTwo> bone_chain = queue[0];
			queue.pop_front();
			Vector<int32_t> bones = bone_chain->get_bones();
			print_line("Chain " + itos(count));
			for (int32_t bone_i = 0; bone_i < bones.size(); bone_i++) {
				String bone_name = stack->skeleton->get_bone_name(bones[bone_i]);
				print_line("Bone " + bone_name);
				if (bone_i < bones.size() - 1) {
					print_line(" - ");
				} else {
					print_line("");
				}
			}
			Vector<Ref<BoneChainTwo>> bone_chains = bone_chain->get_child_chains();
			for (int32_t i = 0; i < bone_chains.size(); i++) {
				queue.push_back(bone_chains[i]);
			}
			count++;
		}
	}
	void register_effectors(Skeleton3D *p_skeleton) {
		ERR_FAIL_COND(!p_skeleton);
		Ref<BoneChainTwo> bone_chain_two;
		bone_chain_two.instance();
		BoneId bone = p_skeleton->find_bone(root_bone);
		bone_chain_two->init(p_skeleton, multi_effector, nullptr, bone);
		bone_chain_two->filter_and_merge_child_chains();
		print_bone_chains(bone_chain_two);
		_change_notify();
		emit_changed();
		emit_signal("ik_changed");
	}
	void add_effector(String p_name, NodePath p_node = NodePath(), Transform p_transform = Transform(), real_t p_budget = 4.0f);
	void register_constraint(Skeleton3D *p_skeleton);
	void set_constraint_count(int32_t p_value);
	int32_t get_constraint_count() const;
	Vector<Ref<BoneEffector>> get_bone_effectors() const;
	int32_t find_effector(String p_name);
	void remove_effector(int32_t p_index);
	int32_t find_constraint(String p_name);
	void set_effector_count(int32_t p_value);
	int32_t get_effector_count() const;
	Ref<BoneEffector> get_effector(int32_t p_index) const;
	void set_effector(int32_t p_index, Ref<BoneEffector> p_effector);
	void set_constraint(int32_t p_index, Ref<KusudamaConstraint> p_constraint);
	Ref<KusudamaConstraint> get_constraint(int32_t p_index) const;
	String get_root_bone() const { return root_bone; }
	void set_root_bone(String p_root_bone) { root_bone = p_root_bone; }
	SkeletonModification3DDMIK();
	~SkeletonModification3DDMIK();

private:
	/**
     * The default maximum number of radians a bone is allowed to rotate per solver iteration.
     * The lower this value, the more natural the pose results. However, this will  the number of iterations
     * the solver requires to converge.
     *
     * !!THIS IS AN EXPENSIVE OPERATION.
     * This updates the entire armature's cache of precomputed quadrance angles.
     * The cache makes things faster in general, but if you need to dynamically change the dampening during a call to IKSolver, use
     * the IKSolver(bone, dampening, iterations, stabilizationPasses) function, which clamps rotations on the fly.
     * @param damp
     */
	static void set_default_dampening(Ref<BoneChain> r_chain, float p_damp);
	static void update_armature_segments(Ref<BoneChain> r_chain);
	static void update_optimal_rotation_to_target_descendants(
			Skeleton3D *p_skeleton,
			Ref<BoneChainItem> p_chain_item,
			float p_dampening,
			bool p_is_translate,
			Vector<Vector3> p_localized_tip_headings,
			Vector<Vector3> p_localized_target_headings,
			Vector<real_t> p_weights,
			Ref<QCP> p_qcp_orientation_aligner,
			int p_iteration,
			float p_total_iterations);
	static void recursively_update_bone_segment_map_from(Ref<BoneChain> r_chain, Ref<BoneChainItem> p_start_from);
	static void QCPSolver(
			Skeleton3D *p_skeleton,
			Ref<BoneChain> p_chain,
			float p_dampening,
			bool p_inverse_weighting,
			int p_stabilization_passes,
			int p_iteration,
			float p_total_iterations);
	static bool build_chain(Ref<DMIKTask> p_task);
	static void update_chain(const Skeleton3D *p_sk, Ref<BoneChainItem> p_chain_item);
	static void solve_simple(Ref<DMIKTask> p_task, bool p_solve_magnet);

public:
	static const int32_t x_axis = 0;
	static const int32_t y_axis = 1;
	static const int32_t z_axis = 2;

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
			Ref<BoneChain> r_chain,
			Ref<BoneChainItem> p_for_bone,
			float p_dampening,
			bool p_translate,
			int p_stabilization_passes,
			int p_iteration,
			int p_total_iterations);

	static float
	get_manual_msd(Vector<Vector3> &r_localized_effector_headings, Vector<Vector3> &r_localized_target_headings,
			const Vector<real_t> &p_weights);

	static void update_target_headings(Ref<BoneChain> r_chain, Vector<Vector3> &r_localized_target_headings,
			Vector<real_t> p_weights, Transform p_bone_xform);

	static void update_effector_headings(Ref<BoneChain> r_chain, Vector<Vector3> &r_localized_effector_headings,
			Transform p_bone_xform);

	static Ref<DMIKTask> create_simple_task(Skeleton3D *p_sk, const Transform &goal_transform,
			float p_dampening = -1, int p_stabilizing_passes = -1,
			Ref<SkeletonModification3DDMIK> p_constraints = NULL);

	static void make_goal(Ref<DMIKTask> p_task, const Transform &p_inverse_transf, float blending_delta);

	static void solve(Ref<DMIKTask> p_task, float blending_delta, bool override_effector_basis, bool p_use_magnet,
			const Vector3 &p_magnet_position);
};

#endif //MULTI_CONSTRAINT_H
