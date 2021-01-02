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

#ifndef bone_chain_item_h__
#define bone_chain_item_h__

#include "bone_chain_target.h"
#include "bone_effector.h"
#include "bone_effector_transform.h"
#include "core/object/reference.h"

class EWBIKBoneChainTarget;
class SkeletonModification3DEWBIK;
class Skeleton3D;
class KusudamaConstraint;
class PhysicalBone3D;

class EWBIKSegmentedSkeleton3D : public Reference {
	GDCLASS(EWBIKSegmentedSkeleton3D, Reference);
	friend class SkeletonModification3DEWBIK;
	friend class EWBIKBoneChainTarget;
	friend class KusudamaConstraint;

private:
	Vector<Ref<EWBIKBoneEffector>> multi_effector;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> child_chains;

	bool processed = false;
	bool aligned = false;
	Ref<EWBIKSegmentedSkeleton3D> chain_root = nullptr;
	Vector<Ref<EWBIKBoneChainTarget>> targets;
	Vector<Vector3> localized_target_headings;
	Vector<Vector3> localized_effector_headings;
	Vector<real_t> weights;
	Ref<SkeletonModification3DEWBIK> mod = nullptr;
	float dampening = Math::deg2rad(5.0f);
	Map<int, Ref<EWBIKSegmentedSkeleton3D>> bone_segment_map;
	// TODO expose through ui
	int ik_iterations = 1;
	// TODO expose through ui
	int stabilization_passes = 4;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> children;
	Ref<EWBIKSegmentedSkeleton3D> parent_item = nullptr;
	int bone = -1;
	PhysicalBone3D *pb = nullptr;
	bool ik_orientation_lock = false;
	float stiffness_scalar = 0.0f;
	float bone_height = 0.0f;
	float length = 0.0f;
	Ref<EWBIKSegmentedSkeleton3D> base_bone;
	Ref<EWBIKSegmentedSkeleton3D> tip_bone;
	Skeleton3D *skeleton = nullptr;
	//contains the parentChain of this bone chain, if any.
	Ref<EWBIKSegmentedSkeleton3D> parent_chain;
	//will be set to true if this chain or any of its descendants have an effector.
	//a post processing step will remove any chains which are not active
	bool is_active = false;
	//will be set to true if the tip of this chain is an effector.
	bool has_effector = false;

public:
	void recursively_align_axes_outward_from(Ref<EWBIKSegmentedSkeleton3D> p_bone);

	/**aligns this bone and all relevant childBones to their coresponding simulatedAxes (if any) in the SegmentedArmature
	 * @param b bone to start from
	 */
	void recursively_align_bones_to_sim_axes_from(Ref<EWBIKSegmentedSkeleton3D> p_bone);
	void align_axes_to_bones();
	void set_processed(bool p_b);
	Ref<EWBIKSegmentedSkeleton3D> find_child(const int p_bone_id);
	Ref<EWBIKSegmentedSkeleton3D> add_child(const int p_bone_id);
	void update_cos_dampening();
	void set_axes_to_returned(Transform p_global, Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen,
			float p_angle_dampen);
	void set_axes_to_be_snapped(Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen);
	void rootwardly_update_falloff_cache_from(Ref<EWBIKSegmentedSkeleton3D> p_current);
	bool is_bone_effector(Ref<EWBIKSegmentedSkeleton3D> current_bone);
	void build_chain(Ref<EWBIKSegmentedSkeleton3D> p_start_from);
	void create_child_chains(Ref<EWBIKSegmentedSkeleton3D> p_from_bone);
	void remove_inactive_children();
	void merge_with_child_if_appropriate();
	void print_bone_chains(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_current_chain);
	Vector<Ref<EWBIKSegmentedSkeleton3D>> get_bone_children(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_bone);
	Vector<StringName> get_default_effectors(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_bone_chain, Ref<EWBIKSegmentedSkeleton3D> p_current_chain);
	bool is_chain_active() const;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> get_child_chains();
	Vector<Ref<EWBIKSegmentedSkeleton3D>> get_bones();
	void init(Skeleton3D *p_skeleton, Ref<SkeletonModification3DEWBIK> p_mod, Vector<Ref<EWBIKBoneEffector>> p_multi_effector, Ref<EWBIKSegmentedSkeleton3D> p_chain, Ref<EWBIKSegmentedSkeleton3D> p_parent_chain, Ref<EWBIKSegmentedSkeleton3D> p_base_bone);
	/**sets this bone chain and all of its ancestors to active */
	void set_active();
	/**
     * Remove any child chains which are not active.
     *
     * if this chain's tip doesn't have effectors, and it only has one active child chain,
     * it also merges this chain with the child chain.
     *
     * This is done recursively.
     * @return
     */
	void filter_and_merge_child_chains();
	void recursively_create_penalty_array(Ref<EWBIKSegmentedSkeleton3D> from, Vector<Vector<real_t>> &r_weight_array, Vector<Ref<EWBIKSegmentedSkeleton3D>> pin_sequence, float current_falloff);
	int get_default_iterations() const;
	void create_headings_arrays();
	void force_update_bone_children_transforms(Ref<EWBIKSegmentedSkeleton3D> p_current_chain);
};

#endif // bone_chain_item_h__
