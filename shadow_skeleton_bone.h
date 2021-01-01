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

class EWBIKShadowSkeletonBone : public Reference {
	GDCLASS(EWBIKShadowSkeletonBone, Reference);
	friend class SkeletonModification3DEWBIK;
	friend class EWBIKBoneChainTarget;
	friend class KusudamaConstraint;

private:
	Vector<Ref<EWBIKBoneEffector>> multi_effector;
	Vector<Ref<EWBIKShadowSkeletonBone>> child_chains;

	bool processed = false;
	bool aligned = false;
	Ref<EWBIKShadowSkeletonBone> chain_root = nullptr;
	Vector<Ref<EWBIKBoneChainTarget>> targets;
	Vector<Vector3> localized_target_headings;
	Vector<Vector3> localized_effector_headings;
	Vector<real_t> weights;
	Ref<SkeletonModification3DEWBIK> mod = nullptr;
	float dampening = Math::deg2rad(5.0f);
	Map<int, Ref<EWBIKShadowSkeletonBone>> bone_segment_map;
	// TODO expose through ui
	int ik_iterations = 1;
	// TODO expose through ui
	int stabilization_passes = 4;
	Vector<Ref<EWBIKShadowSkeletonBone>> children;
	Ref<EWBIKShadowSkeletonBone> parent_item = nullptr;
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
	Ref<KusudamaConstraint> constraint = nullptr;
	Ref<EWBIKShadowSkeletonBone> base_bone;
	Ref<EWBIKShadowSkeletonBone> tip_bone;
	Skeleton3D *skeleton = nullptr;
	//contains the parentChain of this bone chain, if any.
	Ref<EWBIKShadowSkeletonBone> parent_chain;
	//will be set to true if this chain or any of its descendants have an effector.
	//a post processing step will remove any chains which are not active
	bool is_active = false;
	//will be set to true if the tip of this chain is an effector.
	bool has_effector = false;

public:
	void recursively_align_axes_outward_from(Ref<EWBIKShadowSkeletonBone> b);
	/**
	 * aligns all simulation axes from this root of this chain up until the pinned tips
	 * of any child chains with the constraint are local axes of their corresponding bone. 
	 */
	void align_axes_to_bones();
	void set_processed(bool p_b);
	Ref<EWBIKShadowSkeletonBone> find_child(const int p_bone_id);
	Ref<EWBIKShadowSkeletonBone> add_child(const int p_bone_id);
	void update_cos_dampening();
	void
	set_axes_to_returned(Transform p_global, Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen,
			float p_angle_dampen);
	void set_axes_to_be_snapped(Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen);
	void populate_return_dampening_iteration_array(Ref<KusudamaConstraint> k);
	void rootwardly_update_falloff_cache_from(Ref<EWBIKShadowSkeletonBone> p_current);
	bool is_bone_effector(Ref<EWBIKShadowSkeletonBone> current_bone);
	void build_chain(Ref<EWBIKShadowSkeletonBone> p_start_from);
	void create_child_chains(Ref<EWBIKShadowSkeletonBone> p_from_bone);
	void remove_inactive_children();
	void merge_with_child_if_appropriate();
	void print_bone_chains(Skeleton3D *p_skeleton, Ref<EWBIKShadowSkeletonBone> p_current_chain);
	Vector<Ref<EWBIKShadowSkeletonBone>> get_bone_children(Skeleton3D *p_skeleton, Ref<EWBIKShadowSkeletonBone> p_bone);
	Vector<StringName> get_default_effectors(Skeleton3D *p_skeleton, Ref<EWBIKShadowSkeletonBone> p_bone_chain, Ref<EWBIKShadowSkeletonBone> p_current_chain);
	bool is_chain_active() const;
	Vector<Ref<EWBIKShadowSkeletonBone>> get_child_chains();
	Vector<Ref<EWBIKShadowSkeletonBone>> get_bones();
	void init(Skeleton3D *p_skeleton, Ref<SkeletonModification3DEWBIK> p_mod, Vector<Ref<EWBIKBoneEffector>> p_multi_effector, Ref<EWBIKShadowSkeletonBone> p_chain, Ref<EWBIKShadowSkeletonBone> p_parent_chain, Ref<EWBIKShadowSkeletonBone> p_base_bone);
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
	void recursively_create_penalty_array(Ref<EWBIKShadowSkeletonBone> from, Vector<Vector<real_t>> &r_weight_array, Vector<Ref<EWBIKShadowSkeletonBone>> pin_sequence, float current_falloff);
	int get_default_iterations() const;
	void create_headings_arrays();
	void force_update_bone_children_transforms(Ref<EWBIKShadowSkeletonBone> p_current_chain);
};

#endif // bone_chain_item_h__
