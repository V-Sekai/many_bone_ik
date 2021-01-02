/*************************************************************************/
/*  bone_chain_item.cpp                                                  */
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

#include "shadow_skeleton_bone.h"
#include "bone_effector.h"
#include "scene/3d/skeleton_3d.h"
#include "skeleton_modification_3d_ewbik.h"

void EWBIKSegmentedSkeleton3D::recursively_align_axes_outward_from(Ref<EWBIKSegmentedSkeleton3D> b) {
	if (b->base_bone.is_null()) {
		return;
	}
	Ref<EWBIKState> state = b->mod->get_state();
	state->align_shadow_bone_globals_to(b->bone, state->get_shadow_pose_local(b->bone));
	state->mark_dirty(b->bone);
	state->force_update_bone_children_transforms(b->bone);
	state->align_shadow_constraint_globals_to(b->bone, state->get_shadow_constraint_axes_local(b->bone));
	state->mark_dirty(b->bone);
	state->force_update_bone_children_transforms(b->bone);
	Vector<Ref<EWBIKSegmentedSkeleton3D>> children = b->get_bone_children(skeleton, b);
	for (int32_t child_i = 0; child_i < children.size(); child_i++) {
		b->recursively_align_axes_outward_from(children[child_i]);
	}
}

void EWBIKSegmentedSkeleton3D::recursively_align_bones_to_sim_axes_from(Ref<EWBIKSegmentedSkeleton3D> p_bone) {
	ERR_FAIL_COND(p_bone.is_null());
	Ref<EWBIKSegmentedSkeleton3D> chain = p_bone->base_bone;
	if (chain.is_null()) {
		return;
	}
	Ref<EWBIKState> state = p_bone->mod->get_state();
	if (p_bone->parent_chain.is_null()) {
		Transform simulatedLocalAxes = state->get_shadow_pose_local(p_bone->bone);
		state->align_shadow_bone_globals_to(p_bone->bone, simulatedLocalAxes);
	} else {
		Transform simulatedLocalAxes = state->get_shadow_pose_local(p_bone->bone);
		state->rotate_to(p_bone->bone, simulatedLocalAxes.basis.get_rotation_quat());
		state->mark_dirty(p_bone->bone);
		state->force_update_bone_children_transforms(p_bone->bone);
	}
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bone_children = p_bone->get_bone_children(skeleton, p_bone);
	for (int32_t bone_i = 0; bone_i < bone_children.size(); bone_i++) {
		recursively_align_bones_to_sim_axes_from(bone_children[bone_i]);
	}
	chain->aligned = false;
	chain->processed = false;
}

void EWBIKSegmentedSkeleton3D::align_axes_to_bones() {
	if (!is_bone_effector(base_bone) && parent_chain.is_valid()) {
		parent_chain->align_axes_to_bones();
	} else {
		recursively_align_axes_outward_from(base_bone);
	}
}

void EWBIKSegmentedSkeleton3D::set_processed(bool p_b) {
	processed = p_b;
	if (processed == false) {
		Vector<Ref<EWBIKSegmentedSkeleton3D>> chains = get_child_chains();
		for (int32_t i = 0; i < chains.size(); i++) {
			chains.write[i]->set_processed(false);
		}
	}
}

bool EWBIKSegmentedSkeleton3D::is_bone_effector(Ref<EWBIKSegmentedSkeleton3D> current_bone) {
	bool is_effector = false;
	for (int32_t i = 0; i < multi_effector.size(); i++) {
		Ref<EWBIKBoneEffector> effector = multi_effector[i];
		if (effector.is_null()) {
			continue;
		}
		StringName bone_name = skeleton->get_bone_name(current_bone->bone);
		if (effector->get_name() == bone_name) {
			is_effector = true;
			break;
		}
	}
	return is_effector;
}

void EWBIKSegmentedSkeleton3D::build_chain(Ref<EWBIKSegmentedSkeleton3D> p_start_from) {
	Ref<EWBIKSegmentedSkeleton3D> current_bone = p_start_from;
	Ref<EWBIKState> state;
	state.instance();
	mod->set_state(state);
	mod->get_state()->init(mod);
	while (true) {
		Vector<Ref<EWBIKSegmentedSkeleton3D>> current_bone_children = get_bone_children(skeleton, current_bone);
		children.push_back(current_bone);
		tip_bone = current_bone;
		current_bone->mod = mod;
		Ref<KusudamaConstraint> constraint = mod->get_state()->get_constraint(current_bone->bone);
		mod->get_state()->get_constraint(current_bone->bone);
		current_bone->pb = skeleton->get_physical_bone(current_bone->bone);
		if (current_bone_children.size() != 1 || is_bone_effector(current_bone)) {
			create_child_chains(current_bone);
			if (is_bone_effector(current_bone)) {
				has_effector = true;
				set_active();
			}
			break;
		}
		current_bone = current_bone_children[0]; //by definition, there is only one child to this bone if we reach this line.
	}
}

Vector<Ref<EWBIKSegmentedSkeleton3D>> EWBIKSegmentedSkeleton3D::get_bone_children(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_bone) {
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bone_chain_items;
	for (int32_t bone_i = 0; bone_i < p_skeleton->get_bone_count(); bone_i++) {
		int32_t parent = p_skeleton->get_bone_parent(bone_i);
		if (parent == p_bone->bone) {
			if (bone_chain_items.find(p_bone) == -1) {
				Ref<EWBIKSegmentedSkeleton3D> item;
				item.instance();
				item->bone = bone_i;
				chain_root->bone_segment_map[bone_i] = item;
				item->chain_root = chain_root;
				item->parent_item = chain_root->bone_segment_map[parent];
				bone_chain_items.push_back(item);
			}
		}
	}
	return bone_chain_items;
}

void EWBIKSegmentedSkeleton3D::create_child_chains(Ref<EWBIKSegmentedSkeleton3D> p_from_bone) {
	Vector<Ref<EWBIKSegmentedSkeleton3D>> children = get_bone_children(skeleton, p_from_bone);
	for (int i = 0; i < children.size(); i++) {
		Ref<EWBIKSegmentedSkeleton3D> child = children[i];
		child->init(skeleton, mod, multi_effector, chain_root, this, child);
		child_chains.push_back(child);
	}
}

void EWBIKSegmentedSkeleton3D::remove_inactive_children() {
	Vector<Ref<EWBIKSegmentedSkeleton3D>> new_child_chains;
	for (int i = 0; i < child_chains.size(); i++) {
		if (child_chains[i]->is_chain_active()) {
			new_child_chains.push_back(child_chains[i]);
		}
	}
	child_chains = new_child_chains;
}

void EWBIKSegmentedSkeleton3D::merge_with_child_if_appropriate() {
	if (child_chains.size() == 1 && !has_effector) {
		Ref<EWBIKSegmentedSkeleton3D> child = child_chains[0];
		tip_bone = child->tip_bone;
		has_effector = child->has_effector;
		children.append_array(child->children);
		child_chains = child->child_chains;
		remove_inactive_children();
	}
}

void EWBIKSegmentedSkeleton3D::print_bone_chains(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_current_chain) {
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bones = p_current_chain->get_bones();
	ERR_FAIL_COND(!p_current_chain->is_chain_active());
	print_line("Chain");
	for (int32_t bone_i = 0; bone_i < bones.size(); bone_i++) {
		StringName bone_name = p_skeleton->get_bone_name(bones[bone_i]->bone);
		print_line("Bone " + bone_name);
		if (bone_i < bones.size() - 1) {
			print_line(" - ");
		} else {
			print_line("");
		}
	}
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bone_chains = p_current_chain->get_child_chains();
	for (int32_t i = 0; i < bone_chains.size(); i++) {
		print_bone_chains(p_skeleton, bone_chains[i]);
	}
}

Vector<StringName> EWBIKSegmentedSkeleton3D::get_default_effectors(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_bone_chain, Ref<EWBIKSegmentedSkeleton3D> p_current_chain) {
	Vector<StringName> effectors;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bones = p_current_chain->get_bones();
	BoneId bone = p_current_chain->tip_bone->bone;
	StringName bone_name = p_skeleton->get_bone_name(bone);
	effectors.push_back(bone_name);
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bone_chains = p_current_chain->get_child_chains();
	for (int32_t bone_chain_i = 0; bone_chain_i < bone_chains.size(); bone_chain_i++) {
		effectors.append_array(get_default_effectors(p_skeleton, p_bone_chain, bone_chains[bone_chain_i]));
	}
	return effectors;
}

bool EWBIKSegmentedSkeleton3D::is_chain_active() const {
	return is_active;
}

Vector<Ref<EWBIKSegmentedSkeleton3D>> EWBIKSegmentedSkeleton3D::get_bones() {
	return children;
}

Vector<Ref<EWBIKSegmentedSkeleton3D>> EWBIKSegmentedSkeleton3D::get_child_chains() {
	return child_chains;
}

void EWBIKSegmentedSkeleton3D::init(Skeleton3D *p_skeleton, Ref<SkeletonModification3DEWBIK> p_mod, Vector<Ref<EWBIKBoneEffector>> p_multi_effector, Ref<EWBIKSegmentedSkeleton3D> p_chain, Ref<EWBIKSegmentedSkeleton3D> p_parent_chain, Ref<EWBIKSegmentedSkeleton3D> p_base_bone) {
	ERR_FAIL_COND(this == parent_chain.ptr());
	parent_chain = p_parent_chain;
	base_bone = p_base_bone;
	skeleton = p_skeleton;
	multi_effector = p_multi_effector;
	chain_root = p_chain;
	mod = p_mod;
	build_chain(p_base_bone);
}

void EWBIKSegmentedSkeleton3D::set_active() {
	is_active = true;
	if (parent_chain.is_valid()) {
		parent_chain->set_active();
	}
}

void EWBIKSegmentedSkeleton3D::filter_and_merge_child_chains() {
	remove_inactive_children();
	merge_with_child_if_appropriate();
	for (int i = 0; i < child_chains.size(); i++) {
		child_chains.write[i]->filter_and_merge_child_chains();
	}
}