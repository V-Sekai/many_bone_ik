/*************************************************************************/
/*  skeleton_modification_3d_ewbik.cpp                                   */
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

#include "skeleton_modification_3d_ewbik.h"
#include "core/templates/map.h"
#include "ik_bone_3d.h"
#include "ik_effector_3d.h"

int32_t SkeletonModification3DEWBIK::get_ik_iterations() const {
	return ik_iterations;
}

void SkeletonModification3DEWBIK::set_ik_iterations(int32_t p_iterations) {
	ERR_FAIL_COND_MSG(p_iterations <= 0, "EWBIK max iterations must be at least one. Set enabled to false to disable the EWBIK simulation.");
	ik_iterations = p_iterations;
}

String SkeletonModification3DEWBIK::get_root_bone() const {
	return root_bone;
}

void SkeletonModification3DEWBIK::set_root_bone(const String &p_root_bone) {
	root_bone = p_root_bone;
	if (skeleton) {
		root_bone_index = skeleton->find_bone(root_bone);
	}

	is_dirty = true;
}

BoneId SkeletonModification3DEWBIK::get_root_bone_index() const {
	return root_bone_index;
}

void SkeletonModification3DEWBIK::set_root_bone_index(BoneId p_index) {
	root_bone_index = p_index;
	if (skeleton) {
		root_bone = skeleton->get_bone_name(p_index);
	}
	is_dirty = true;
}

void SkeletonModification3DEWBIK::add_effector(const String &p_name, const NodePath &p_target_node, bool p_use_node_rot) {
	Ref<IKBone3D> effector_bone = Ref<IKBone3D>(memnew(IKBone3D(p_name, skeleton, nullptr, get_default_damp())));
	Ref<IKEffector3D> effector = Ref<IKEffector3D>(memnew(IKEffector3D(effector_bone)));
	effector->set_target_node(p_target_node, skeleton);
	effector->call_deferred("update_target_cache", skeleton);
	effector_bone->set_effector(effector);
	effector_count++;
	is_dirty = true;
}

Ref<IKBone3D> SkeletonModification3DEWBIK::find_effector(const String &p_bone) const {
	BoneId bone = skeleton->find_bone(p_bone);
	if (!effectors_map.has(bone)) {
		return Ref<IKBone3D>();
	}
	return effectors_map[bone];
}

void SkeletonModification3DEWBIK::remove_effector(const String &p_name) {
	Ref<IKBone3D> bone = find_effector(p_name);
	if (bone.is_null()) {
		return;
	}
	bone->remove_effector();
	is_dirty = true;
	update_effectors_map();
	notify_property_list_changed();
}

void SkeletonModification3DEWBIK::_execute(float delta) {
	ERR_FAIL_COND_MSG(!stack || !is_setup || skeleton == nullptr,
			"Modification is not setup and therefore cannot execute!");
	if (!enabled) {
		return;
	}

	if (is_dirty) {
		update_skeleton();
		return;
	}
	solve(stack->get_strength());
	execution_error_found = false;
}

void SkeletonModification3DEWBIK::_setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;
	if (!stack) {
		return;
	}
	skeleton = stack->skeleton;
	if (!skeleton) {
		return;
	}

	if (root_bone.is_empty()) {
		Vector<int32_t> roots;
		for (int32_t bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
			int32_t parent = skeleton->get_bone_parent(bone_i);
			if (parent == -1) {
				roots.push_back(bone_i);
			}
		}
		if (roots.size()) {
			set_root_bone_index(roots[0]);
		}
	} else if (root_bone_index == -1) {
		set_root_bone(root_bone);
	}
	ERR_FAIL_COND(root_bone.is_empty());

	is_setup = true;
	execution_error_found = false;
	notify_property_list_changed();
}

void SkeletonModification3DEWBIK::solve(real_t p_blending_delta) {
	if (p_blending_delta <= 0.01f) {
		return; // Skip solving
	}

	if (effector_count && segmented_skeleton.is_valid() && segmented_skeleton->get_effector_direct_descendents_size() > 0) {
		update_shadow_bones_transform();
		iterated_improved_solver(get_default_damp());
		update_skeleton_bones_transform(p_blending_delta);
	}
}

void SkeletonModification3DEWBIK::iterated_improved_solver(real_t p_damp) {
	ERR_FAIL_NULL(segmented_skeleton);
	for (int i = 0; i < ik_iterations; i++) {
		segmented_skeleton->grouped_segment_solver(constraint_stabilization_passes, p_damp);
	}
}

void SkeletonModification3DEWBIK::update_skeleton() {
	if (!is_dirty) {
		return;
	}
	if (!skeleton) {
		return;
	}
	if (effector_count) {
		update_segments();
	} else {
		generate_default_effectors();
	}
	if (segmented_skeleton.is_null()) {
		return;
	}
	segmented_skeleton->update_effector_list();
	is_dirty = false;
}

void SkeletonModification3DEWBIK::generate_default_effectors() {
	segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, root_bone_index)));
	segmented_skeleton->generate_default_segments_from_root();
	Vector<Ref<IKBoneChain>> effector_chains = segmented_skeleton->get_effector_direct_descendents();
	effector_count = effector_chains.size();
	update_effectors_map();
	update_bone_list(get_debug_skeleton());
}

void SkeletonModification3DEWBIK::update_shadow_bones_transform() {
	for (int32_t bone_i = 0; bone_i < bone_list.size(); bone_i++) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		bone->set_initial_transform(skeleton);
	}
}

void SkeletonModification3DEWBIK::update_skeleton_bones_transform(real_t p_blending_delta) {
	for (int32_t bone_i = 0; bone_i < bone_list.size(); bone_i++) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		bone->set_skeleton_bone_transform(skeleton, p_blending_delta);
	}
}

void SkeletonModification3DEWBIK::update_segments() {
	if (effector_count) {
		update_effectors_map();
		segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, root_bone_index, effectors_map)));
		update_bone_list();
	}
}

void SkeletonModification3DEWBIK::update_bone_list(bool p_debug_skeleton) {
	bone_list.clear();
	ERR_FAIL_NULL(segmented_skeleton);
	segmented_skeleton->get_bone_list(bone_list, true, p_debug_skeleton);
	bone_list.reverse();
}

void SkeletonModification3DEWBIK::update_effectors_map() {
	effectors_map.clear();
	for (int32_t index = 0; index < bone_list.size(); index++) {
		Ref<IKBone3D> effector_bone = bone_list[index];
		if (!effector_bone->is_effector()) {
			continue;
		}
		effector_bone->get_effector()->update_target_cache(skeleton);
		effectors_map[effector_bone->get_bone_id()] = effector_bone;
	}
}

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &SkeletonModification3DEWBIK::get_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_ik_iterations", "iterations"), &SkeletonModification3DEWBIK::set_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_root_bone", "root_bone"), &SkeletonModification3DEWBIK::set_root_bone);
	ClassDB::bind_method(D_METHOD("get_root_bone"), &SkeletonModification3DEWBIK::get_root_bone);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node", "budget"), &SkeletonModification3DEWBIK::add_effector);
	ClassDB::bind_method(D_METHOD("update_skeleton"), &SkeletonModification3DEWBIK::update_skeleton);
	ClassDB::bind_method(D_METHOD("get_debug_skeleton"), &SkeletonModification3DEWBIK::get_debug_skeleton);
	ClassDB::bind_method(D_METHOD("set_debug_skeleton", "enabled"), &SkeletonModification3DEWBIK::set_debug_skeleton);
	ClassDB::bind_method(D_METHOD("get_default_damp"), &SkeletonModification3DEWBIK::get_default_damp);
	ClassDB::bind_method(D_METHOD("set_default_damp", "damp"), &SkeletonModification3DEWBIK::set_default_damp);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "print_skeleton"), "set_debug_skeleton", "get_debug_skeleton");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "root_bone"), "set_root_bone", "get_root_bone");
}

SkeletonModification3DEWBIK::SkeletonModification3DEWBIK() {
	enabled = true;
}

SkeletonModification3DEWBIK::~SkeletonModification3DEWBIK() {
}

float SkeletonModification3DEWBIK::get_default_damp() const {
	return default_damp;
}

void SkeletonModification3DEWBIK::set_default_damp(float p_default_damp) {
	default_damp = p_default_damp;
}

bool SkeletonModification3DEWBIK::get_debug_skeleton() const {
	return debug_skeleton;
}

void SkeletonModification3DEWBIK::set_debug_skeleton(bool p_enabled) {
	debug_skeleton = p_enabled;
	update_bone_list(true);
}
