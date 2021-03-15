/*************************************************************************/
/*  ewbik_segmented_skeleton_3d.cpp                                      */
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

#include "ewbik_segmented_skeleton_3d.h"

Ref<EWBIKShadowBone3D> EWBIKSegmentedSkeleton3D::get_root() const {
	return root;
}
Ref<EWBIKShadowBone3D> EWBIKSegmentedSkeleton3D::get_tip() const {
	return tip;
}

int32_t EWBIKSegmentedSkeleton3D::get_chain_length() const {
	return chain_length;
}

bool EWBIKSegmentedSkeleton3D::is_root_effector() const {
	return root->is_effector();
}

bool EWBIKSegmentedSkeleton3D::is_tip_effector() const {
	return tip->is_effector();
}

Vector<Ref<EWBIKSegmentedSkeleton3D>> EWBIKSegmentedSkeleton3D::get_effector_descendents() const {
	return effector_descendents;
}

int32_t EWBIKSegmentedSkeleton3D::get_effector_descendents_size() const {
	return effector_descendents.size();
}

BoneId EWBIKSegmentedSkeleton3D::find_root_bone_id(BoneId p_bone) {
	BoneId root_id = p_bone;
	while (skeleton->get_bone_parent(root_id) != -1)
	{
		root_id = skeleton->get_bone_parent(root_id);
	}

	return root_id;
}

void EWBIKSegmentedSkeleton3D::generate_skeleton_segments(const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map) {
	child_chains.clear();

	Ref<EWBIKShadowBone3D> tempTip = root;
	chain_length = 0;
	while (true) {
		chain_length++;
		Vector<BoneId> children_with_effector_descendants = tempTip->get_children_with_effector_descendants(skeleton, p_map);
		if (children_with_effector_descendants.size() > 1 || tempTip->is_effector()) {
			tip = tempTip;
			for (int32_t child_i = 0; child_i < children_with_effector_descendants.size(); child_i++) {
				BoneId child_bone = children_with_effector_descendants[child_i];
				child_chains.push_back(Ref<EWBIKSegmentedSkeleton3D>(memnew(EWBIKSegmentedSkeleton3D(skeleton, child_bone, p_map, tip))));
			}
			break;
		} else if (children_with_effector_descendants.size() == 1) {
			BoneId bone_id = children_with_effector_descendants[0];
			if (p_map.has(bone_id)) {
				Ref<EWBIKShadowBone3D> next = p_map[bone_id];
				next->set_parent(tempTip);
				tempTip = next;
			} else {
				Ref<EWBIKShadowBone3D> next = Ref<EWBIKShadowBone3D>(memnew(EWBIKShadowBone3D(bone_id, tempTip)));
				tempTip = next;
			}
		} else {
			tip = tempTip;
			break;
		}
	}
	update_effector_descendents();
	generate_segment_maps();
}

void EWBIKSegmentedSkeleton3D::update_effector_descendents() {
	effector_descendents.clear();
	if (is_tip_effector()) {
		effector_descendents.push_back(this);
	} else {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			Ref<EWBIKSegmentedSkeleton3D> child_segment = child_chains[child_i];
			effector_descendents.append_array(child_segment->get_effector_descendents());
		}
	}
}

void EWBIKSegmentedSkeleton3D::generate_segment_maps() {
	bones_map.clear();
	Ref<EWBIKShadowBone3D> current_bone = tip;
	Ref<EWBIKShadowBone3D> stop_on = root;
	while (current_bone.is_valid()) {
		bones_map[current_bone->get_bone_id()] = current_bone;
		if (current_bone == stop_on)
			break;
		current_bone = current_bone->get_parent();
	}
}

void EWBIKSegmentedSkeleton3D::generate_default_segments_from_root() {
	child_chains.clear();

	Ref<EWBIKShadowBone3D> tempTip = root;
	chain_length = 0;
	while (true) {
		chain_length++;
		Vector<BoneId> children = skeleton->get_bone_children(tempTip->get_bone_id());
		if (children.size() > 1) {
			tip = tempTip;
			for (int32_t child_i = 0; child_i < children.size(); child_i++) {
				BoneId child_bone = children[child_i];
				Ref<EWBIKSegmentedSkeleton3D> child_segment = Ref<EWBIKSegmentedSkeleton3D>(memnew(EWBIKSegmentedSkeleton3D(skeleton, child_bone, tip)));
				child_segment->generate_default_segments_from_root();
				child_chains.push_back(child_segment);
			}
			break;
		} else if (children.size() == 1) {
			BoneId bone_id = children[0];
			Ref<EWBIKShadowBone3D> next = Ref<EWBIKShadowBone3D>(memnew(EWBIKShadowBone3D(bone_id, tempTip)));
			tempTip = next;
		} else {
			tip = tempTip;
			tip->create_effector();
			break;
		}

	}
	update_effector_descendents();
	generate_segment_maps();
}

void EWBIKSegmentedSkeleton3D::align_axes_to_bones() {
	if (!is_root_effector() && parent_chain.is_valid()) {
		parent_chain->align_axes_to_bones();
	} else {
		recursively_align_axes_outward_from(root);
	}
}

void EWBIKSegmentedSkeleton3D::recursively_align_axes_outward_from(Ref<EWBIKShadowBone3D> p_bone) {
	Ref<EWBIKSegmentedSkeleton3D> bchain = get_child_segment_containing(p_bone);
	if (bchain.is_valid()) {

	}
}

Ref<EWBIKSegmentedSkeleton3D> EWBIKSegmentedSkeleton3D::get_child_segment_containing(Ref<EWBIKShadowBone3D> p_bone) {
	if (bones_map.has(p_bone->get_bone_id())) {
		return this;
	} else {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			Ref<EWBIKSegmentedSkeleton3D> child_segment = child_chains.write[child_i]->get_child_segment_containing(p_bone);
			if (child_segment.is_valid())
				return child_segment;
		}
	}
	return nullptr;
}

Vector<Ref<EWBIKShadowBone3D>> EWBIKSegmentedSkeleton3D::get_bone_list() const {
	Vector<Ref<EWBIKShadowBone3D>> bones;
	for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
		bones.append_array(child_chains[child_i]->get_bone_list());
	}
	Ref<EWBIKShadowBone3D> current_bone = tip;
	while (current_bone.is_valid()) {
		bones.push_back(current_bone);
		if (current_bone == root)
			break;
		current_bone = current_bone->get_parent();
	}
	return bones;
}

void EWBIKSegmentedSkeleton3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_root_effector"), &EWBIKSegmentedSkeleton3D::is_root_effector);
	ClassDB::bind_method(D_METHOD("is_tip_effector"), &EWBIKSegmentedSkeleton3D::is_tip_effector);
}

EWBIKSegmentedSkeleton3D::EWBIKSegmentedSkeleton3D(Skeleton3D *p_skeleton, BoneId p_root_bone, Ref<EWBIKSegmentedSkeleton3D> p_parent) {
	skeleton = p_skeleton;
	BoneId root_id = find_root_bone_id(p_root_bone);
	root = Ref<EWBIKShadowBone3D>(memnew(EWBIKShadowBone3D(root_id)));
	if (p_parent.is_valid()) {
		parent_chain = p_parent;
		root->set_parent(p_parent->get_tip());
	}
}

EWBIKSegmentedSkeleton3D::EWBIKSegmentedSkeleton3D(Skeleton3D *p_skeleton, BoneId p_root_bone,
		const HashMap<BoneId, Ref<EWBIKShadowBone3D>> &p_map, Ref<EWBIKSegmentedSkeleton3D> p_parent) {
	skeleton = p_skeleton;
	BoneId root_id = find_root_bone_id(p_root_bone);
	if (p_map.has(root_id)) {
		root = p_map[root_id];
	} else {
		root = Ref<EWBIKShadowBone3D>(memnew(EWBIKShadowBone3D(root_id)));
	}
	if (p_parent.is_valid()) {
		parent_chain = p_parent;
		root->set_parent(p_parent->get_tip());
	}
	generate_skeleton_segments(p_map);
}