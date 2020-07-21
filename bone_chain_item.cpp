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

#include "bone_chain_item.h"
#include "bone_effector.h"
#include "scene/3d/skeleton_3d.h"
#include "skeleton_modification_3d_dmik.h"


void BoneChainItem::recursively_align_axes_outward_from(Ref<BoneChainItem> sb) {
	Transform bAxes = sb->axes;
	Transform cAxes = sb->constraint->get_constraint_axes();
	if (sb->base_bone.is_null()) {
		return;
	}
	//bAxes.alignGlobalsTo(b.localAxes());
	//bAxes.markDirty();
	//bAxes.updateGlobal();
	//cAxes.alignGlobalsTo(b.getMajorRotationAxes());
	//cAxes.markDirty();
	//cAxes.updateGlobal();
	Vector<Ref<BoneChainItem>> bones = sb->base_bone->get_bones();
	for (int32_t bones_i = 0; bones_i < bones.size(); bones_i++) {
		//bAxes.alignGlobalsTo(b.localAxes());
		//bAxes.markDirty();
		//bAxes.updateGlobal();
		//cAxes.alignGlobalsTo(b.getMajorRotationAxes());
		//cAxes.markDirty();
		//cAxes.updateGlobal();
	}
	Vector<Ref<BoneChainItem>> children = sb->get_child_chains();
	for (int32_t child_i = 0; child_i < children.size(); child_i++) {
		sb->recursively_align_axes_outward_from(children[child_i]);
	}
}

void BoneChainItem::align_axes_to_bones() {
	if (!is_bone_effector(base_bone) && parent_chain.is_valid()) {
		parent_chain->align_axes_to_bones();
	} else {
		recursively_align_axes_outward_from(base_bone);
	}
}

void BoneChainItem::set_processed(bool p_b) {
	processed = p_b;
	if (processed == false) {
		Vector<Ref<BoneChainItem>> chains = get_child_chains();
		for (int32_t i = 0; i < chains.size(); i++) {
			chains.write[i]->set_processed(false);
		}
	}
}

bool BoneChainItem::is_bone_effector(Ref<BoneChainItem> current_bone) {
	bool is_effector = false;
	Ref<BoneEffector> effector;
	for (int32_t i = 0; i < multi_effector.size(); i++) {
		effector = multi_effector[i];
		if (effector.is_null()) {
			continue;
		}
		String bone_name = skeleton->get_bone_name(current_bone->bone);
		if (effector->get_name() == bone_name) {
			is_effector = true;
			break;
		}
	}
	return is_effector;
}

void BoneChainItem::build_chain(Ref<BoneChainItem> p_start_from) {
	Ref<BoneChainItem> current_bone = p_start_from;
	while (true) {
		Vector<Ref<BoneChainItem>> current_bone_children = get_bone_children(skeleton, current_bone);
		children.push_back(current_bone);
		tip_bone = current_bone;
		String constraint_name = skeleton->get_bone_name(current_bone->bone);
		int32_t constraint_i = constraints->find_constraint(constraint_name);
		current_bone->constraints = constraints;
		current_bone->constraint = constraints->get_constraint(constraint_i);
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

Vector<Ref<BoneChainItem>> BoneChainItem::get_bone_children(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_bone) {
	Vector<Ref<BoneChainItem>> bone_chain_items;
	for (int32_t bone_i = 0; bone_i < p_skeleton->get_bone_count(); bone_i++) {
		int32_t parent = p_skeleton->get_bone_parent(bone_i);
		if (parent == p_bone->bone) {
			if (bone_chain_items.find(p_bone) == -1) {
				Ref<BoneChainItem> item;
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

void BoneChainItem::create_child_chains(Ref<BoneChainItem> p_from_bone) {
	Vector<Ref<BoneChainItem>> children = get_bone_children(skeleton, p_from_bone);
	for (int i = 0; i < children.size(); i++) {
		Ref<BoneChainItem> child = children[i];
		child->init(skeleton, constraints, multi_effector, chain_root, this, child);
		child_chains.push_back(child);
	}
}

void BoneChainItem::remove_inactive_children() {
	Vector<Ref<BoneChainItem>> new_child_chains;
	for (int i = 0; i < child_chains.size(); i++) {
		if (child_chains[i]->is_chain_active()) {
			new_child_chains.push_back(child_chains[i]);
		}
	}
	child_chains = new_child_chains;
}

void BoneChainItem::merge_with_child_if_appropriate() {
	if (child_chains.size() == 1 && !has_effector) {
		Ref<BoneChainItem> child = child_chains[0];
		tip_bone = child->tip_bone;
		has_effector = child->has_effector;
		children.append_array(child->children);
		child_chains = child->child_chains;
		remove_inactive_children();
	}
}

void BoneChainItem::print_bone_chains(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_current_chain) {
	Vector<Ref<BoneChainItem>> bones = p_current_chain->get_bones();
	ERR_FAIL_COND(!p_current_chain->is_chain_active());
	print_line("Chain");
	for (int32_t bone_i = 0; bone_i < bones.size(); bone_i++) {
		String bone_name = p_skeleton->get_bone_name(bones[bone_i]->bone);
		print_line("Bone " + bone_name);
		if (bone_i < bones.size() - 1) {
			print_line(" - ");
		} else {
			print_line("");
		}
	}
	Vector<Ref<BoneChainItem>> bone_chains = p_current_chain->get_child_chains();
	for (int32_t i = 0; i < bone_chains.size(); i++) {
		print_bone_chains(p_skeleton, bone_chains[i]);
	}
}

Vector<String> BoneChainItem::get_default_effectors(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_bone_chain, Ref<BoneChainItem> p_current_chain) {
	Vector<String> effectors;
	Vector<Ref<BoneChainItem>> bones = p_current_chain->get_bones();
	BoneId bone = p_current_chain->tip_bone->bone;
	String bone_name = p_skeleton->get_bone_name(bone);
	effectors.push_back(bone_name);
	Vector<Ref<BoneChainItem>> bone_chains = p_current_chain->get_child_chains();
	for (int32_t i = 0; i < bone_chains.size(); i++) {
		effectors.append_array(get_default_effectors(p_skeleton, p_bone_chain, bone_chains[i]));
	}
	return effectors;
}

bool BoneChainItem::is_chain_active() const {
	return is_active;
}

Vector<Ref<BoneChainItem>> BoneChainItem::get_bones() {
	return children;
}

Vector<Ref<BoneChainItem>> BoneChainItem::get_child_chains() {
	return child_chains;
}

void BoneChainItem::init(Skeleton3D *p_skeleton, Ref<SkeletonModification3DDMIK> p_constraints, Vector<Ref<BoneEffector>> p_multi_effector, Ref<BoneChainItem> p_chain, Ref<BoneChainItem> p_parent_chain, Ref<BoneChainItem> p_base_bone) {
	ERR_FAIL_COND(this == parent_chain.ptr());
	parent_chain = p_parent_chain;
	base_bone = p_base_bone;
	skeleton = p_skeleton;
	multi_effector = p_multi_effector;
	chain_root = p_chain;
	constraints = p_constraints;
	build_chain(p_base_bone);
}

void BoneChainItem::set_active() {
	is_active = true;
	if (parent_chain.is_valid()) {
		parent_chain->set_active();
	}
}

void BoneChainItem::filter_and_merge_child_chains() {
	remove_inactive_children();
	merge_with_child_if_appropriate();
	for (int i = 0; i < child_chains.size(); i++) {
		child_chains.write[i]->filter_and_merge_child_chains();
	}
}
