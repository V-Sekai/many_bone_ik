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

#pragma once

#include "core/reference.h"
#include "bone_effector.h"
#include "bone_chain_target.h"

class BoneChainTarget;
class SkeletonModification3DDMIK;
class Skeleton3D;
class KusudamaConstraint;
class PhysicalBone3D;

class BoneEffectorTransform : public Reference {
	GDCLASS(BoneEffectorTransform, Reference);

public:
	int effector_bone = -1;
	Transform goal_transform;
};

class BoneChainItem : public Reference {
	GDCLASS(BoneChainItem, Reference);

private:
	Vector<Ref<BoneEffector>> multi_effector;
	Vector<Ref<BoneChainItem>> child_chains;

public:
	Transform axes;
	Transform axes_global;
	Ref<BoneChainItem> chain_root = nullptr;
	Vector<Ref<BoneChainTarget>> targets;
	Vector<Vector3> localized_target_headings;
	Vector<Vector3> localized_effector_headings;
	Vector<real_t> weights;
	Ref<SkeletonModification3DDMIK> constraints = nullptr;
	float dampening = Math::deg2rad(5.0f);
	Map<int, Ref<BoneChainItem>> bone_segment_map;
	// TODO expose through ui
	int ik_iterations = 15;
	Vector<Ref<BoneChainItem>> children;
	Ref<BoneChainItem> parent_item = nullptr;
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
	Ref<BoneChainItem> base_bone;
	Ref<BoneChainItem> tip_bone;
	Skeleton3D *skeleton = nullptr;
	//contains the parentChain of this bone chain, if any.
	Ref<BoneChainItem> parent_chain;
	//will be set to true if this chain or any of its descendants have an effector.
	//a post processing step will remove any chains which are not active
	bool is_active = false;
	//will be set to true if the tip of this chain is an effector.
	bool has_effector = false;
	BoneChainItem();
	float get_bone_height() const;
	void set_bone_height(const float p_bone_height);
	Ref<BoneChainItem> find_child(const int p_bone_id);
	Ref<BoneChainItem> add_child(const int p_bone_id);
	void set_stiffness(float p_stiffness);
	float get_stiffness() const;
	void update_cos_dampening();
	void
	set_axes_to_returned(Transform p_global, Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen,
			float p_angle_dampen);
	void set_axes_to_be_snapped(Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen);
	void populate_return_dampening_iteration_array(Ref<KusudamaConstraint> k);
	void rootwardly_update_falloff_cache_from(Ref<BoneChainItem> p_current);
	bool is_bone_effector(Ref<BoneChainItem> current_bone);
	void build_chain(Ref<BoneChainItem> p_start_from);
	void create_child_chains(Ref<BoneChainItem> p_from_bone);
	void remove_inactive_children();
	void merge_with_child_if_appropriate();
	void print_bone_chains(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_current_chain);
	Vector<Ref<BoneChainItem>> get_bone_children(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_bone);
	Vector<String> get_default_effectors(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_bone_chain, Ref<BoneChainItem> p_current_chain);
	bool is_chain_active() const;
	Vector<Ref<BoneChainItem>> get_child_chains();
	Vector<Ref<BoneChainItem>> get_bones();
	void init(Skeleton3D *p_skeleton, Ref<SkeletonModification3DDMIK> p_constraints, Vector<Ref<BoneEffector>> p_multi_effector, Ref<BoneChainItem> p_chain, Ref<BoneChainItem> p_parent_chain, Ref<BoneChainItem> p_base_bone);
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
	void recursively_create_penalty_array(Ref<BoneChainItem> from, Vector<Vector<real_t>> &r_weight_array, Vector<Ref<BoneChainItem>> pin_sequence, float current_falloff);
	int get_default_iterations() const;
	void create_headings_arrays();
	void force_update_bone_children_transforms(Ref<BoneChainItem> p_current_chain, Ref<BoneChainItem> p_bone);
};