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

int32_t SkeletonModification3DEWBIK::get_ik_iterations() const {
	return ik_iterations;
}

void SkeletonModification3DEWBIK::set_ik_iterations(int32_t p_iterations) {
	ERR_FAIL_COND_MSG(p_iterations <= 0, "EWBIK max iterations must be at least one. Set enabled to false to disable the EWBIK simulation.");
	ik_iterations = p_iterations;
	calc_done = false;
}

StringName SkeletonModification3DEWBIK::get_root_bone_name() const {
	return root_bone_name;
}

void SkeletonModification3DEWBIK::set_root_bone_name(StringName p_bone_name) {
	root_bone_name = p_bone_name;
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_target_count(int32_t p_value) {
	multi_effector.resize(p_value);
	for (int32_t i = effector_count; i < p_value; i++) {
		Ref<IKBone3D> bone = Ref<IKBone3D>(memnew(IKBone3D()));
		bone->create_effector();
		multi_effector.write[i] = bone;
	}
	effector_count = p_value;
	is_dirty = true;

	notify_property_list_changed();
}

int32_t SkeletonModification3DEWBIK::get_target_count() const {
	return effector_count;
}

void SkeletonModification3DEWBIK::add_effector(const String &p_name, const NodePath &p_target_node, bool p_use_node_rot,
		const Transform &p_target_xform) {
	Ref<IKBone3D> effector_bone = Ref<IKBone3D>(memnew(IKBone3D(p_name, skeleton)));
	Ref<IKEffector3D> effector = Ref<IKEffector3D>(memnew(IKEffector3D(effector_bone)));
	effector->set_target_node(p_target_node);
	effector->set_use_target_node_rotation(p_use_node_rot);
	effector->set_target_transform(p_target_xform);
	effector_bone->set_effector(effector);
	multi_effector.push_back(effector_bone);
	effector_count++;

	is_dirty = true;
	notify_property_list_changed();
}

Ref<IKBone3D> SkeletonModification3DEWBIK::get_effector(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_effector.size(), NULL);
	Ref<IKBone3D> effector = multi_effector[p_index];
	return effector;
}

void SkeletonModification3DEWBIK::set_effector(int32_t p_index, const Ref<IKBone3D> &p_effector) {
	ERR_FAIL_COND(p_effector.is_null());
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.write[p_index] = p_effector;
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_effector_bone_name(int32_t p_effector_index, StringName p_bone_name) {
	multi_effector.write[p_effector_index]->set_bone_name(skeleton, p_bone_name);
	is_dirty = true;
}

StringName SkeletonModification3DEWBIK::get_effector_bone_name(int32_t p_effector_index) const {
	StringName bone_name = multi_effector[p_effector_index]->get_bone_name();
	ERR_FAIL_COND_V(!skeleton, "");
	BoneId bone = skeleton->find_bone(bone_name);
	ERR_FAIL_COND_V(bone == -1, "");
	String name = skeleton->get_bone_name(bone);
	return name;
}

void SkeletonModification3DEWBIK::set_effector_nodepath(int32_t p_index, const NodePath &p_target_node) {
	multi_effector.write[p_index]->get_effector()->set_target_node(p_target_node);
	calc_done = false;
}

NodePath SkeletonModification3DEWBIK::get_effector_nodepath(int32_t p_index) const {
	return multi_effector[p_index]->get_effector()->get_target_node();
}

void SkeletonModification3DEWBIK::set_effector_transform(int32_t p_index, const Transform &p_target_transform) {
	multi_effector.write[p_index]->get_effector()->set_target_transform(p_target_transform);
	calc_done = false;
}

Transform SkeletonModification3DEWBIK::get_effector_transform(int32_t p_index) const {
	return multi_effector[p_index]->get_effector()->get_target_transform();
}

void SkeletonModification3DEWBIK::set_effector_use_node_rotation(int32_t p_index, bool p_use_node_rot) {
	multi_effector.write[p_index]->get_effector()->set_use_target_node_rotation(p_use_node_rot);
	calc_done = false;
}

bool SkeletonModification3DEWBIK::get_effector_use_node_rotation(int32_t p_index) const {
	return multi_effector[p_index]->get_effector()->get_use_target_node_rotation();
}

Vector<Ref<IKBone3D>> SkeletonModification3DEWBIK::get_bone_effectors() const {
	return multi_effector;
}

int32_t SkeletonModification3DEWBIK::find_target(const String &p_name) const {
	for (int32_t effector_i = 0; effector_i < multi_effector.size(); effector_i++) {
		if (multi_effector[effector_i].is_valid() && multi_effector[effector_i]->get_bone_name() == p_name) {
			return effector_i;
		}
	}
	return -1;
}

void SkeletonModification3DEWBIK::remove_target(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.remove(p_index);
	effector_count--;

	is_dirty = true;
}

void SkeletonModification3DEWBIK::execute(float delta) {
	ERR_FAIL_COND_MSG(!stack || !is_setup || skeleton == nullptr,
			"Modification is not setup and therefore cannot execute!");
	if (!enabled) {
		return;
	}

	if (is_dirty) {
		update_skeleton();
	}
	if (!is_calc_done()) {
		solve(stack->get_strength());
	}
	execution_error_found = false;
}

void SkeletonModification3DEWBIK::setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;
	if (!stack) {
		return;
	}
	skeleton = stack->skeleton;
	if (!skeleton) {
		return;
	}
	if (!root_bone_name) {
		_calculate_root_bone();
	}
	ERR_FAIL_COND(!root_bone_name);

	is_dirty = true;
	is_setup = true;
	calc_done = false;
	call_deferred("update_skeleton");

	execution_error_found = false;
}

void SkeletonModification3DEWBIK::solve(real_t p_blending_delta) {
	if (p_blending_delta <= 0.01f) {
		return; // Skip solving
	}

	if (effector_count && segmented_skeleton.is_valid() && segmented_skeleton->get_effector_direct_descendents_size() > 0) {
		update_shadow_bones_transform();
		iterated_improved_solver();
		update_skeleton_bones_transform(p_blending_delta);
	}

	calc_done = true;
}

void SkeletonModification3DEWBIK::iterated_improved_solver() {
	ERR_FAIL_NULL(segmented_skeleton);
	for (int i = 0; i < ik_iterations; i++) {
		segmented_skeleton->grouped_segment_solver(constraint_stabilization_passes);
	}
}

void SkeletonModification3DEWBIK::update_skeleton() {
	if (!is_dirty)
		return;

	if (effector_count) {
		update_segments();
	} else {
		generate_default_effectors();
	}
	segmented_skeleton->update_effector_list();
	notify_property_list_changed();

	is_dirty = false;
	calc_done = false;
}

void SkeletonModification3DEWBIK::generate_default_effectors() {
	ERR_FAIL_COND(!skeleton);
	BoneId root_bone = skeleton->find_bone(root_bone_name);
	ERR_FAIL_COND(root_bone == -1);
	segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, root_bone)));
	segmented_skeleton->generate_default_segments_from_root();
	Vector<Ref<IKBoneChain>> effector_chains = segmented_skeleton->get_effector_direct_descendents();
	effector_count = effector_chains.size();
	multi_effector.resize(effector_count);
	for (int32_t chain_i = 0; chain_i < effector_count; chain_i++) {
		multi_effector.write[chain_i] = effector_chains[chain_i]->get_tip();
		Ref<IKBoneChain> segment = effector_chains[chain_i];
	}
	update_effectors_map();
	update_bone_list(get_debug_skeleton());
}

void SkeletonModification3DEWBIK::update_shadow_bones_transform() {
	// Reset the local bone overrides
	for (int32_t bone_i = 0; bone_i < bone_list.size(); bone_i++) {
		BoneId bone = skeleton->find_bone(bone_list[bone_i]->get_bone_name());
		skeleton->set_bone_local_pose_override(bone,
				Transform(), 0.0, false);
	}

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
		ERR_FAIL_COND(!skeleton);
		BoneId root_bone = skeleton->find_bone(root_bone_name);
		ERR_FAIL_COND(root_bone == -1);
		segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, root_bone, effectors_map)));
		update_bone_list(false);
	}
}

void SkeletonModification3DEWBIK::update_bone_list(bool p_debug_skeleton) {
	bone_list.clear();
	ERR_FAIL_NULL(segmented_skeleton);
	segmented_skeleton->get_bone_list(bone_list, p_debug_skeleton);
	bone_list.reverse();
}

void SkeletonModification3DEWBIK::update_effectors_map() {
	effectors_map.clear();
	for (int32_t index = 0; index < effector_count; index++) {
		Ref<IKBone3D> effector_bone = multi_effector[index];
		BoneId bone = skeleton->find_bone(effector_bone->get_bone_name());
		effectors_map[bone] = effector_bone;
	}
}

bool SkeletonModification3DEWBIK::is_calc_done() {
	if (!calc_done) {
		return false;
	}

	for (int32_t ei = 0; ei < multi_effector.size(); ei++) {
		Ref<IKEffector3D> effector = multi_effector[ei]->get_effector();
		if (effector->is_node_xform_changed(skeleton)) {
			calc_done = false;
			return false;
		}
	}

	return true;
}

void SkeletonModification3DEWBIK::_validate_property(PropertyInfo &property) const {
}

void SkeletonModification3DEWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	PropertyInfo root_bone_info = PropertyInfo(Variant::STRING_NAME, "root_bone");
	if (skeleton) {
		String names = "None";
		for (int bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
			names += ",";
			names += skeleton->get_bone_name(bone_i);
		}
		root_bone_info.hint = PROPERTY_HINT_ENUM;
		root_bone_info.hint_string = names;
	} else {
		root_bone_info.hint = PROPERTY_HINT_NONE;
		root_bone_info.hint_string = "";
	}
	p_list->push_back(root_bone_info);

	p_list->push_back(PropertyInfo(Variant::INT, "ik_iterations", PROPERTY_HINT_RANGE, "0,65535,1"));
	p_list->push_back(PropertyInfo(Variant::INT, "target_count", PROPERTY_HINT_RANGE, "0,65535,1"));
	for (int effector_i = 0; effector_i < effector_count; effector_i++) {
		PropertyInfo effector_name;
		effector_name.type = Variant::STRING;
		effector_name.name = "effectors/" + itos(effector_i) + "/name";
		if (skeleton) {
			String names = "None";
			for (int bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
				names += ",";
				names += skeleton->get_bone_name(bone_i);
			}
			effector_name.hint = PROPERTY_HINT_ENUM;
			effector_name.hint_string = names;
		} else {
			effector_name.hint = PROPERTY_HINT_NONE;
			effector_name.hint_string = "";
		}
		p_list->push_back(effector_name);
		p_list->push_back(
				PropertyInfo(Variant::NODE_PATH, "effectors/" + itos(effector_i) + "/target_node"));
		p_list->push_back(
				PropertyInfo(Variant::BOOL, "effectors/" + itos(effector_i) + "/use_node_rotation"));
		p_list->push_back(
				PropertyInfo(Variant::TRANSFORM, "effectors/" + itos(effector_i) + "/target_transform"));
	}
}

bool SkeletonModification3DEWBIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "ik_iterations") {
		r_ret = get_ik_iterations();
		return true;
	} else if (name == "target_count") {
		r_ret = get_target_count();
		return true;
	} else if (name == "root_bone") {
		r_ret = get_root_bone_name();
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, get_target_count(), false);
		ERR_FAIL_COND_V(get_effector(index).is_null(), false);
		if (what == "name") {
			r_ret = get_effector_bone_name(index);
			return true;
		} else if (what == "target_node") {
			r_ret = get_effector_nodepath(index);
			return true;
		} else if (what == "use_node_rotation") {
			r_ret = get_effector_use_node_rotation(index);
			return true;
		} else if (what == "target_transform") {
			r_ret = get_effector_transform(index);
			return true;
		}
	}

	return false;
}

bool SkeletonModification3DEWBIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "ik_iterations") {
		set_ik_iterations(p_value);
		return true;
	} else if (name == "target_count") {
		set_target_count(p_value);
		return true;
	} else if (name == "root_bone") {
		if (!String(p_value).is_empty()) {
			set_root_bone_name(p_value);
			return true;
		}
		_calculate_root_bone();
		ERR_FAIL_COND_V(!root_bone_name, false);
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, effector_count, false);
		Ref<IKBone3D> target = get_effector(index);
		if (target.is_null()) {
			target.instance();
			target->create_effector();
			set_effector(index, target);
		}
		if (what == "name") {
			set_effector_bone_name(index, p_value);
			return true;
		} else if (what == "target_node") {
			set_effector_nodepath(index, p_value);

			return true;
		} else if (what == "use_node_rotation") {
			set_effector_use_node_rotation(index, p_value);

			return true;
		} else if (what == "target_transform") {
			set_effector_transform(index, p_value);

			return true;
		}
	}

	return false;
}

void SkeletonModification3DEWBIK::_calculate_root_bone() {
	Vector<int32_t> roots;
	for (int32_t bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
		int32_t parent = skeleton->get_bone_parent(bone_i);
		if (parent == -1) {
			roots.push_back(bone_i);
		}
	}
	if (roots.size()) {
		String bone_name = skeleton->get_bone_name(roots[0]);
		ERR_FAIL_COND(bone_name.is_empty());
		root_bone_name = bone_name;
	}
}

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &SkeletonModification3DEWBIK::get_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_ik_iterations", "iterations"), &SkeletonModification3DEWBIK::set_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_root_bone_name", "name"), &SkeletonModification3DEWBIK::set_root_bone_name);
	ClassDB::bind_method(D_METHOD("get_root_bone_name"), &SkeletonModification3DEWBIK::get_root_bone_name);
	ClassDB::bind_method(D_METHOD("get_effector_count"), &SkeletonModification3DEWBIK::get_target_count);
	ClassDB::bind_method(D_METHOD("set_effector_count", "count"),
			&SkeletonModification3DEWBIK::set_target_count);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node", "target_transform", "budget"), &SkeletonModification3DEWBIK::add_effector);
	ClassDB::bind_method(D_METHOD("get_effector", "index"), &SkeletonModification3DEWBIK::get_effector);
	ClassDB::bind_method(D_METHOD("set_effector", "index", "effector"), &SkeletonModification3DEWBIK::set_effector);
	ClassDB::bind_method(D_METHOD("update_skeleton"), &SkeletonModification3DEWBIK::update_skeleton);
	ClassDB::bind_method(D_METHOD("get_debug_skeleton"), &SkeletonModification3DEWBIK::get_debug_skeleton);
	ClassDB::bind_method(D_METHOD("set_debug_skeleton", "enabled"), &SkeletonModification3DEWBIK::set_debug_skeleton);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_skeleton"), "set_debug_skeleton", "get_debug_skeleton");
}

SkeletonModification3DEWBIK::SkeletonModification3DEWBIK() {
	enabled = true;
}

SkeletonModification3DEWBIK::~SkeletonModification3DEWBIK() {
}

bool SkeletonModification3DEWBIK::get_debug_skeleton() const {
	return debug_skeleton;
}

void SkeletonModification3DEWBIK::set_debug_skeleton(bool p_enabled) {
	debug_skeleton = p_enabled;
	update_bone_list(true);
}
