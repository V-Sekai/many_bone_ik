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

void SkeletonModification3DEWBIK::set_effector_count(int32_t p_value) {
	multi_effector.resize(p_value);
	for (int32_t i = effector_count; i < p_value; i++) {
		multi_effector.write[i].instantiate();
	}
	effector_count = p_value;
	is_dirty = true;

	notify_property_list_changed();
}

int32_t SkeletonModification3DEWBIK::get_effector_count() const {
	return effector_count;
}

void SkeletonModification3DEWBIK::add_effector(const String &p_name, const NodePath &p_target_node, const bool &p_use_node_rotation) {
	int32_t count = get_effector_count();
	set_effector_count(count + 1);
	set_effector_bone(count, p_name);
	set_effector_target_nodepath(count, p_target_node);
	set_effector_use_node_rotation(count, p_use_node_rotation);

	is_dirty = true;
	notify_property_list_changed();
}

Ref<IKBone3D> SkeletonModification3DEWBIK::find_effector(const String &p_name) const {
	if (!skeleton) {
		return Ref<IKBone3D>();
	}
	BoneId bone = skeleton->find_bone(p_name);
	if (bone == -1) {
		return Ref<IKBone3D>();
	}
	if (!effectors_map.has(bone)) {
		return Ref<IKBone3D>();
	}
	return effectors_map[bone];
}

void SkeletonModification3DEWBIK::set_effector_bone(int32_t p_effector_index, const String &p_bone) {
	Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	data->set_name(p_bone);
	is_dirty = true;
	notify_property_list_changed();
}

void SkeletonModification3DEWBIK::set_effector_target_nodepath(int32_t p_effector_index, const NodePath &p_target_node) {
	Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	ERR_FAIL_NULL(data);
	data->target_node = p_target_node;
	is_dirty = true;
	notify_property_list_changed();
}

NodePath SkeletonModification3DEWBIK::get_effector_target_nodepath(int32_t p_effector_index) {
	ERR_FAIL_INDEX_V(p_effector_index, multi_effector.size(), NodePath());
	const Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	return data->target_node;
}

void SkeletonModification3DEWBIK::set_effector_use_node_rotation(int32_t p_effector_index, bool p_use_node_rot) {
	Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	ERR_FAIL_NULL(data);
	data->use_target_node_rotation = p_use_node_rot;
	is_dirty = true;
	notify_property_list_changed();
}

bool SkeletonModification3DEWBIK::get_effector_use_node_rotation(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, multi_effector.size(), false);
	const Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	return data->use_target_node_rotation;
}

Vector<Ref<IKEffector3DData>> SkeletonModification3DEWBIK::get_bone_effectors() const {
	return multi_effector;
}

void SkeletonModification3DEWBIK::remove_effector(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.remove_at(p_index);
	effector_count--;
	multi_effector.resize(effector_count);

	is_dirty = true;
	notify_property_list_changed();
}

void SkeletonModification3DEWBIK::_execute(real_t delta) {
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
		segmented_skeleton->grouped_segment_solver(p_damp);
	}
}

void SkeletonModification3DEWBIK::update_skeleton() {
	if (!is_dirty) {
		return;
	}
	if (!skeleton) {
		return;
	}
	segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, root_bone_index)));
	segmented_skeleton->generate_default_segments_from_root();
	update_segments();
	segmented_skeleton->update_effector_list();
	update_bone_list(debug_skeleton);
	is_dirty = false;
}

void SkeletonModification3DEWBIK::update_shadow_bones_transform() {
	for (int32_t bone_i = 0; bone_i < bone_list.size(); bone_i++) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		bone->set_initial_transform(skeleton);
	}
}

void SkeletonModification3DEWBIK::update_skeleton_bones_transform(real_t p_blending_delta) {
	for (int32_t bone_i = 0; bone_i < bone_list.size(); bone_i++) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		bone->set_skeleton_bone_transform(skeleton, p_blending_delta);
	}
}

void SkeletonModification3DEWBIK::update_segments() {
	if (!effector_count) {
		return;
	}
	segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, root_bone_index, effectors_map)));
	segmented_skeleton->generate_default_segments_from_root();
	update_bone_list();
	update_effectors_map();
}

void SkeletonModification3DEWBIK::update_bone_list(bool p_debug_skeleton) {
	bone_list.clear();
	ERR_FAIL_NULL(segmented_skeleton);
	segmented_skeleton->get_bone_list(bone_list, true, p_debug_skeleton);
	bone_list.reverse();
}

void SkeletonModification3DEWBIK::update_effectors_map() {
	effectors_map.clear();
	ERR_FAIL_NULL(skeleton);

	for (int effector_i = 0; effector_i < get_effector_count(); effector_i++) {
		Ref<IKEffector3DData> data = multi_effector.write[effector_i];
		String bone = data->get_name();
		BoneId bone_id = skeleton->find_bone(bone);
		Vector3 priority = data->priority;
		float depth_falloff = data->depth_falloff;
		ERR_CONTINUE(bone_id == -1);
		Ref<IKBone3D> ik_bone_3d = segmented_skeleton->find_bone(bone_id);
		ERR_FAIL_NULL(ik_bone_3d);
		if (!ik_bone_3d->is_effector()) {
			ik_bone_3d->create_effector();
		}
		effectors_map[ik_bone_3d->get_bone_id()] = ik_bone_3d;
		Ref<IKEffector3D> effector_3d = ik_bone_3d->get_effector();
		effector_3d->set_target_node(skeleton, data->target_node);
		effector_3d->update_target_cache(skeleton);
		effector_3d->set_priority(priority);
		effector_3d->set_depth_falloff(depth_falloff);
	}
	is_dirty = true;
}

void SkeletonModification3DEWBIK::_validate_property(PropertyInfo &property) const {
	if (property.name == "root_bone") {
		if (skeleton) {
			String names = "None";
			for (int i = 0; i < skeleton->get_bone_count(); i++) {
				names += ",";
				names += skeleton->get_bone_name(i);
			}

			property.hint = PROPERTY_HINT_ENUM;
			property.hint_string = names;
		} else {
			property.hint = PROPERTY_HINT_NONE;
			property.hint_string = "";
		}
	}
}

void SkeletonModification3DEWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "ik_iterations", PROPERTY_HINT_RANGE, "0,65535,1"));
	p_list->push_back(PropertyInfo(Variant::FLOAT, "default_damp", PROPERTY_HINT_RANGE, "0.01,179.99,0.01,degrees"));
	p_list->push_back(PropertyInfo(Variant::INT, "effector_count", PROPERTY_HINT_RANGE, "0,65535,1"));
	for (int i = 0; i < effector_count; i++) {
		PropertyInfo effector_name;
		effector_name.type = Variant::STRING;
		effector_name.name = "effectors/" + itos(i) + "/name";
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
				PropertyInfo(Variant::NODE_PATH, "effectors/" + itos(i) + "/target_node"));

		p_list->push_back(PropertyInfo(Variant::BOOL, "effectors/" + itos(i) + "/use_node_rotation"));
		p_list->push_back(
				PropertyInfo(Variant::VECTOR3, "effectors/" + itos(i) + "/priority"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "effectors/" + itos(i) + "/depth_falloff"));
		p_list->push_back(
				PropertyInfo(Variant::BOOL, "effectors/" + itos(i) + "/remove"));
	}
}

bool SkeletonModification3DEWBIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "ik_iterations") {
		r_ret = get_ik_iterations();
		return true;
	} else if (name == "default_damp") {
		r_ret = get_default_damp();
		return true;
	} else if (name == "effector_count") {
		r_ret = get_effector_count();
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, multi_effector.size(), false);
		Ref<IKEffector3DData> data = multi_effector[index];
		ERR_FAIL_NULL_V(data, false);
		if (what == "name") {
			r_ret = data->get_name();
			return true;
		} else if (what == "target_node") {
			r_ret = data->target_node;
			return true;
		} else if (what == "use_node_rotation") {
			r_ret = get_effector_use_node_rotation(index);
			return true;
		} else if (what == "priority") {
			r_ret = get_effector_priority(index);
			return true;
		} else if (what == "depth_falloff") {
			r_ret = get_effector_depth_falloff(index);
			return true;
		} else if (what == "remove") {
			r_ret = false;
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
	} else if (name == "default_damp") {
		set_default_damp(p_value);
		return true;
	} else if (name == "effector_count") {
		set_effector_count(p_value);
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, effector_count, true);
		Ref<IKEffector3DData> data = multi_effector.write[index];
		if (what == "name") {
			set_effector_bone(index, p_value);
			return true;
		} else if (what == "target_node") {
			set_effector_target_nodepath(index, p_value);
			return true;
		} else if (what == "use_node_rotation") {
			set_effector_use_node_rotation(index, p_value);

			return true;
		} else if (what == "remove") {
			if (p_value) {
				remove_effector(index);
			}
			return true;
		} else if (what == "priority") {
			set_effector_priority(index, p_value);
			return true;
		} else if (what == "depth_falloff") {
			set_effector_depth_falloff(index, p_value);
			return true;
		}
	}

	return false;
}

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &SkeletonModification3DEWBIK::get_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_ik_iterations", "iterations"), &SkeletonModification3DEWBIK::set_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_root_bone", "root_bone"), &SkeletonModification3DEWBIK::set_root_bone);
	ClassDB::bind_method(D_METHOD("get_root_bone"), &SkeletonModification3DEWBIK::get_root_bone);
	ClassDB::bind_method(D_METHOD("get_effector_count"), &SkeletonModification3DEWBIK::get_effector_count);
	ClassDB::bind_method(D_METHOD("set_effector_count", "count"),
			&SkeletonModification3DEWBIK::set_effector_count);
	ClassDB::bind_method(D_METHOD("remove_effector", "index"),
			&SkeletonModification3DEWBIK::remove_effector);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node"), &SkeletonModification3DEWBIK::add_effector);
	ClassDB::bind_method(D_METHOD("find_effector", "name"), &SkeletonModification3DEWBIK::find_effector);
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
	is_dirty = true;
	notify_property_list_changed();
}

Vector3 SkeletonModification3DEWBIK::get_effector_priority(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, multi_effector.size(), Vector3(0.5, 5.0, 0.0));
	const Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	return data->priority;
}

void SkeletonModification3DEWBIK::set_effector_priority(int32_t p_effector_index, Vector3 p_priority) {
	Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	ERR_FAIL_NULL(data);
	data->priority = p_priority;
	is_dirty = true;
	notify_property_list_changed();
}

float SkeletonModification3DEWBIK::get_effector_depth_falloff(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, multi_effector.size(), 0.0f);
	const Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	return data->depth_falloff;
}

void SkeletonModification3DEWBIK::set_effector_depth_falloff(int32_t p_effector_index, const float p_depth_falloff) {
	Ref<IKEffector3DData> data = multi_effector[p_effector_index];
	ERR_FAIL_NULL(data);
	data->depth_falloff = p_depth_falloff;
	is_dirty = true;
	notify_property_list_changed();
}