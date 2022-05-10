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

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#endif

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
	notify_property_list_changed();
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
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_pin_count(int32_t p_value) {
	pins.resize(p_value);
	for (int32_t i = pin_count; i < p_value; i++) {
		pins.write[i].instantiate();
	}
	pin_count = p_value;
	notify_property_list_changed();
	is_dirty = true;
}

int32_t SkeletonModification3DEWBIK::get_pin_count() const {
	return pin_count;
}

void SkeletonModification3DEWBIK::add_pin(const String &p_name, const NodePath &p_target_node, const bool &p_use_node_rotation) {
	for (Ref<IKEffectorTemplate> pin : pins) {
		if (pin->get_name() == p_name) {
			return;
		}
	}
	int32_t count = get_pin_count();
	set_pin_count(count + 1);
	set_pin_bone(count, p_name);
	set_pin_target_nodepath(count, p_target_node);
	set_pin_use_node_rotation(count, p_use_node_rotation);

	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_pin_bone(int32_t p_pin_index, const String &p_bone) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_pin_index];
	data->set_name(p_bone);
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_pin_target_nodepath(int32_t p_pin_index, const NodePath &p_target_node) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_pin_index];
	ERR_FAIL_NULL(data);
	data->set_target_node(p_target_node);
	notify_property_list_changed();
	is_dirty = true;
}

NodePath SkeletonModification3DEWBIK::get_pin_target_nodepath(int32_t p_pin_index) {
	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), NodePath());
	const Ref<IKEffectorTemplate> data = pins[p_pin_index];
	return data->get_target_node();
}

void SkeletonModification3DEWBIK::set_pin_use_node_rotation(int32_t p_pin_index, bool p_use_node_rot) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_pin_index];
	ERR_FAIL_NULL(data);
	data->set_target_node_rotation(p_use_node_rot);
	notify_property_list_changed();
	is_dirty = true;
}

bool SkeletonModification3DEWBIK::get_pin_use_node_rotation(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), false);
	const Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_target_node_rotation();
}

Vector<Ref<IKEffectorTemplate>> SkeletonModification3DEWBIK::get_bone_effectors() const {
	return pins;
}

void SkeletonModification3DEWBIK::remove_pin(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, pins.size());
	Node *node = skeleton->get_node_or_null(get_pin_target_nodepath(p_index));
	if (node) {
		bool is_tree_exited_connected = node->is_connected(SNAME("tree_exited"), callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath));
		if (is_tree_exited_connected) {
			node->disconnect(SNAME("tree_exited"), callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath));
		}
		bool is_tree_entered_connected = node->is_connected(SNAME("tree_entered"), callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath));
		if (is_tree_entered_connected) {
			node->disconnect(SNAME("tree_entered"), callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath));
		}
		bool is_renamed_connected = node->is_connected(SNAME("renamed"), callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath));
		if (is_renamed_connected) {
			node->disconnect(SNAME("renamed"), callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath));
		}
	}
	pins.remove_at(p_index);
	pin_count--;
	pins.resize(pin_count);
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::_execute(real_t delta) {
	ERR_FAIL_COND_MSG(!stack || !is_setup || skeleton == nullptr,
			"The modification is not set up and therefore cannot execute.");
	if (!enabled) {
		is_dirty = true;
		return;
	}
	if (stack->get_strength() <= 0.01f) {
		execution_error_found = false;
		return;
	}
	if (is_dirty || segmented_skeleton.is_null()) {
		update_skeleton();
	}
	update_shadow_bones_transform();
	double time_ms = OS::get_singleton()->get_ticks_msec() + get_time_budget_millisecond();
	ik_iterations = 0;
	do {
		segmented_skeleton->segment_solver(get_default_damp());
		ik_iterations++;
	} while (time_ms > OS::get_singleton()->get_ticks_msec() && ik_iterations < get_max_ik_iterations());
	set_ik_iterations(ik_iterations);
	update_skeleton_bones_transform(delta);
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
	update_skeleton();
	notify_property_list_changed();
	is_setup = true;
	is_dirty = false;
	execution_error_found = false;
}

void SkeletonModification3DEWBIK::update_skeleton() {
	if (!skeleton) {
		return;
	}
	if (root_bone.is_empty()) {
		Vector<int32_t> roots = skeleton->get_parentless_bones();
		if (roots.size()) {
			set_root_bone_index(roots[0]);
		}
	} else if (root_bone_index == -1) {
		set_root_bone(root_bone);
	}
	ERR_FAIL_COND(root_bone.is_empty());
#ifdef TOOLS_ENABLED
	if (InspectorDock::get_inspector_singleton()->is_connected("edited_object_changed", callable_mp(this, &SkeletonModification3DEWBIK::set_dirty))) {
		InspectorDock::get_inspector_singleton()->disconnect("edited_object_changed", callable_mp(this, &SkeletonModification3DEWBIK::set_dirty));
	}
	InspectorDock::get_inspector_singleton()->connect("edited_object_changed", callable_mp(this, &SkeletonModification3DEWBIK::set_dirty));
#endif
	segmented_skeleton = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, skeleton->get_bone_name(root_bone_index), pins)));
	segmented_skeleton->generate_default_segments_from_root(pins);
	bone_list.clear();
	segmented_skeleton->set_bone_list(bone_list, true, debug_skeleton);
	segmented_skeleton->update_pinned_list();
	for (int effector_i = 0; effector_i < get_pin_count(); effector_i++) {
		Ref<IKEffectorTemplate> data = pins.write[effector_i];
		String bone = data->get_name();
		BoneId bone_id = skeleton->find_bone(bone);
		for (Ref<IKBone3D> ik_bone_3d : bone_list) {
			if (ik_bone_3d->get_bone_id() != bone_id) {
				continue;
			}
			Node *node = skeleton->get_node_or_null(data->get_target_node());
			if (!node) {
				continue;
			}
			Callable callable = callable_mp(this, &SkeletonModification3DEWBIK::set_pin_target_nodepath);
			bool is_tree_exited_connected = node->is_connected(SNAME("tree_exited"), callable);
			if (is_tree_exited_connected) {
				node->disconnect(SNAME("tree_exited"), callable);
			}
			bool is_tree_entered_connected = node->is_connected(SNAME("tree_entered"), callable);
			if (is_tree_entered_connected) {
				node->disconnect(SNAME("tree_entered"), callable);
			}
			bool is_renamed_connected = node->is_connected(SNAME("renamed"), callable);
			if (is_renamed_connected) {
				node->disconnect(SNAME("renamed"), callable);
			}
			node->connect(SNAME("tree_exited"), callable, varray(effector_i, data->get_target_node()));
			node->connect(SNAME("tree_entered"), callable, varray(effector_i, data->get_target_node()));
			node->connect(SNAME("renamed"), callable, varray(effector_i, data->get_target_node()));
		}
	}
	is_dirty = false;
}

void SkeletonModification3DEWBIK::update_shadow_bones_transform() {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		bone->set_initial_pose(skeleton);
		if (bone->is_pinned()) {
			bone->get_pin()->update_goal_global_pose(skeleton);
		}
	}
}

void SkeletonModification3DEWBIK::update_skeleton_bones_transform(real_t p_blending_delta) {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		if (bone->get_bone_id() == -1) {
			continue;
		}
		bone->set_skeleton_bone_pose(skeleton, p_blending_delta);
	}
}

void SkeletonModification3DEWBIK::_validate_property(PropertyInfo &property) const {
	if (property.name == "root_bone") {
		if (skeleton) {
			String names;
			for (int i = 0; i < skeleton->get_bone_count(); i++) {
				String name = skeleton->get_bone_name(i);
				name += ",";
				names += name;
			}
			property.hint = PROPERTY_HINT_ENUM_SUGGESTION;
			property.hint_string = names;
		} else {
			property.hint = PROPERTY_HINT_NONE;
			property.hint_string = "";
		}
	}
}

void SkeletonModification3DEWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "ik_iterations", PROPERTY_HINT_RANGE, "1,60,1,or_greater", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_INTERNAL));
	p_list->push_back(PropertyInfo(Variant::INT, "pin_count", PROPERTY_HINT_RANGE, "0,1024,1", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY, "Pins,pins/"));
	for (int i = 0; i < pin_count; i++) {
		PropertyInfo effector_name;
		effector_name.type = Variant::STRING;
		effector_name.name = "pins/" + itos(i) + "/name";
		if (skeleton) {
			Set<String> existing_pins;
			for (Ref<IKEffectorTemplate> pin : pins) {
				if (pin.is_null()) {
					continue;
				}
				const String name = pin->get_name();
				existing_pins.insert(name);
			}
			String names;
			for (int i = 0; i < skeleton->get_bone_count(); i++) {
				String name = skeleton->get_bone_name(i);
				if (existing_pins.has(name)) {
					continue;
				}
				name += ",";
				names += name;
			}
			effector_name.hint = PROPERTY_HINT_ENUM_SUGGESTION;
			effector_name.hint_string = names;
		} else {
			effector_name.hint = PROPERTY_HINT_NONE;
			effector_name.hint_string = "";
		}
		p_list->push_back(effector_name);
		p_list->push_back(
				PropertyInfo(Variant::NODE_PATH, "pins/" + itos(i) + "/target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"));

		p_list->push_back(PropertyInfo(Variant::BOOL, "pins/" + itos(i) + "/use_node_rotation"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(i) + "/depth_falloff"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(i) + "/priority"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(i) + "/weight_translation"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(i) + "/weight_x_direction"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(i) + "/weight_y_direction"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(i) + "/weight_z_direction"));
	}
	p_list->push_back(PropertyInfo(Variant::INT, "constraint_count", PROPERTY_HINT_RANGE, "0,1024,1", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY, "Constraints,constraints/"));
	for (int constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		PropertyInfo bone_name;
		bone_name.type = Variant::STRING;
		bone_name.name = "constraints/" + itos(constraint_i) + "/name";
		if (skeleton) {
			String names = "";
			for (int bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
				if (skeleton->get_bone_parent(bone_i) == -1) {
					continue;
				}
				String name = skeleton->get_bone_name(bone_i);
				name = ",";
				names += name;
			}
			bone_name.hint = PROPERTY_HINT_ENUM_SUGGESTION;
			bone_name.hint_string = names;
		} else {
			bone_name.hint = PROPERTY_HINT_NONE;
			bone_name.hint_string = "";
		}
		p_list->push_back(bone_name);
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_twist", PROPERTY_HINT_RANGE, "0,359.999999,0.1,degrees"));
		p_list->push_back(
				PropertyInfo(Variant::INT, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone_count",
						PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY,
						vformat("Limit Cones,constraints/%s/kusudama_limit_cone/", itos(constraint_i))));
		for (int cone_i = 0; cone_i < get_kusudama_limit_cone_count(constraint_i); cone_i++) {
			p_list->push_back(
					PropertyInfo(Variant::VECTOR3, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/center", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"));
			p_list->push_back(
					PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/radius"));
		}
	}
}

bool SkeletonModification3DEWBIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "ik_iterations") {
		r_ret = get_ik_iterations();
		return true;
	} else if (name == "pin_count") {
		r_ret = get_pin_count();
		return true;
	} else if (name == "constraint_count") {
		r_ret = get_constraint_count();
		return true;
	} else if (name.begins_with("pins/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, pins.size(), false);
		Ref<IKEffectorTemplate> data = pins[index];
		ERR_FAIL_NULL_V(data, false);
		if (what == "name") {
			r_ret = data->get_name();
			return true;
		} else if (what == "target_node") {
			r_ret = data->get_target_node();
			return true;
		} else if (what == "use_node_rotation") {
			r_ret = get_pin_use_node_rotation(index);
			return true;
		} else if (what == "depth_falloff") {
			r_ret = get_pin_depth_falloff(index);
			return true;
		}
	} else if (name.begins_with("constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, constraint_count, false);
		String begins = "constraints/" + itos(index) + "/kusudama_limit_cone";
		if (what == "name") {
			ERR_FAIL_INDEX_V(index, constraint_names.size(), false);
			r_ret = constraint_names[index];
			return true;
		} else if (what == "kusudama_twist") {
			r_ret = get_kusudama_twist(index);
			return true;
		} else if (what == "kusudama_limit_cone_count") {
			r_ret = get_kusudama_limit_cone_count(index);
			return true;
		} else if (name.begins_with(begins)) {
			int cone_index = name.get_slicec('/', 3).to_int();
			String cone_what = name.get_slicec('/', 4);
			if (cone_what == "center") {
				r_ret = get_kusudama_limit_cone_center(index, cone_index);
				return true;
			} else if (cone_what == "radius") {
				r_ret = get_kusudama_limit_cone_radius(index, cone_index);
				return true;
			}
		}
	}

	return false;
}

bool SkeletonModification3DEWBIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "ik_iterations") {
		set_ik_iterations(p_value);
		return true;
	} else if (name == "pin_count") {
		set_pin_count(p_value);
		return true;
	} else if (name == "constraint_count") {
		set_constraint_count(p_value);
		return true;
	} else if (name.begins_with("pins/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, pin_count, true);
		if (what == "name") {
			set_pin_bone(index, p_value);
			return true;
		} else if (what == "target_node") {
			set_pin_target_nodepath(index, p_value);
			String existing_bone = get_pin_bone_name(index);
			if (!existing_bone.is_empty() && existing_bone != "None") {
				return true;
			}
			String node_path = p_value;
			set_pin_bone(index, node_path.get_file());
			return true;
		} else if (what == "use_node_rotation") {
			set_pin_use_node_rotation(index, p_value);
			return true;
		} else if (what == "depth_falloff") {
			set_pin_depth_falloff(index, p_value);
			return true;
		}
	} else if (name.begins_with("constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, constraint_count, false);
		String begins = "constraints/" + itos(index) + "/kusudama_limit_cone/";
		if (what == "name") {
			ERR_FAIL_INDEX_V(index, constraint_names.size(), false);
			constraint_names.write[index] = p_value;
			return true;
		} else if (what == "kusudama_twist") {
			set_kusudama_twist(index, p_value);
			return true;
		} else if (what == "kusudama_limit_cone_count") {
			set_kusudama_limit_cone_count(index, p_value);
			return true;
		} else if (name.begins_with(begins)) {
			int cone_index = name.get_slicec('/', 3).to_int();
			String cone_what = name.get_slicec('/', 4);
			ERR_FAIL_INDEX_V(cone_index, kusudama_limit_cone_count.size(), false);
			if (cone_what == "center") {
				set_kusudama_limit_cone(index, cone_index, p_value, get_kusudama_limit_cone_radius(index, cone_index));
				return true;
			} else if (cone_what == "radius") {
				set_kusudama_limit_cone(index, cone_index, get_kusudama_limit_cone_center(index, cone_index), p_value);
				return true;
			}
		}
	}
	return false;
}

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_max_ik_iterations"), &SkeletonModification3DEWBIK::get_max_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_max_ik_iterations", "count"), &SkeletonModification3DEWBIK::set_max_ik_iterations);
	ClassDB::bind_method(D_METHOD("get_time_budget_millisecond"), &SkeletonModification3DEWBIK::get_time_budget_millisecond);
	ClassDB::bind_method(D_METHOD("set_time_budget_millisecond", "budget"), &SkeletonModification3DEWBIK::set_time_budget_millisecond);
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &SkeletonModification3DEWBIK::get_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_ik_iterations", "iterations"), &SkeletonModification3DEWBIK::set_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_root_bone", "root_bone"), &SkeletonModification3DEWBIK::set_root_bone);
	ClassDB::bind_method(D_METHOD("get_root_bone"), &SkeletonModification3DEWBIK::get_root_bone);
	ClassDB::bind_method(D_METHOD("get_constraint_count"), &SkeletonModification3DEWBIK::get_constraint_count);
	ClassDB::bind_method(D_METHOD("set_constraint_count", "count"),
			&SkeletonModification3DEWBIK::set_constraint_count);
	ClassDB::bind_method(D_METHOD("get_pin_count"), &SkeletonModification3DEWBIK::get_pin_count);
	ClassDB::bind_method(D_METHOD("set_pin_count", "count"),
			&SkeletonModification3DEWBIK::set_pin_count);
	ClassDB::bind_method(D_METHOD("remove_pin", "index"),
			&SkeletonModification3DEWBIK::remove_pin);
	ClassDB::bind_method(D_METHOD("add_pin", "name", "target_node", "use_node_rotation"), &SkeletonModification3DEWBIK::add_pin);
	ClassDB::bind_method(D_METHOD("get_pin_bone_name", "index"), &SkeletonModification3DEWBIK::get_pin_bone_name);
	ClassDB::bind_method(D_METHOD("update_skeleton"), &SkeletonModification3DEWBIK::update_skeleton);
	ClassDB::bind_method(D_METHOD("get_debug_skeleton"), &SkeletonModification3DEWBIK::get_debug_skeleton);
	ClassDB::bind_method(D_METHOD("set_debug_skeleton", "enabled"), &SkeletonModification3DEWBIK::set_debug_skeleton);
	ClassDB::bind_method(D_METHOD("get_default_damp"), &SkeletonModification3DEWBIK::get_default_damp);
	ClassDB::bind_method(D_METHOD("set_default_damp", "damp"), &SkeletonModification3DEWBIK::set_default_damp);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_ik_iterations", PROPERTY_HINT_RANGE, "1,150,1,or_greater"), "set_max_ik_iterations", "get_max_ik_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "budget_millisecond", PROPERTY_HINT_RANGE, "0.01,2.0,0.01,or_greater"), "set_time_budget_millisecond", "get_time_budget_millisecond");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "print_skeleton", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_debug_skeleton", "get_debug_skeleton");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "root_bone", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_root_bone", "get_root_bone");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "default_damp", PROPERTY_HINT_RANGE, "0.04,179.99,0.01,radians", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_default_damp", "get_default_damp");
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
	notify_property_list_changed();
	is_dirty = true;
}

float SkeletonModification3DEWBIK::get_pin_depth_falloff(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), 0.0f);
	const Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_depth_falloff();
}

void SkeletonModification3DEWBIK::set_pin_depth_falloff(int32_t p_effector_index, const float p_depth_falloff) {
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	ERR_FAIL_NULL(data);
	data->set_depth_falloff(p_depth_falloff);
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_constraint_count(int32_t p_count) {
	int32_t old_count = constraint_count;
	constraint_count = p_count;
	constraint_names.resize(p_count);
	kusudana_twist.resize(p_count);
	kusudama_limit_cone_count.resize(p_count);
	kusudama_limit_cones.resize(p_count);
	for (int32_t constraint_i = p_count; constraint_i-- > old_count;) {
		set_kusudama_twist(constraint_i, 0.0f);
		set_kusudama_limit_cone_count(constraint_i, 0.0f);
	}
	notify_property_list_changed();
	is_dirty = true;
}

int32_t SkeletonModification3DEWBIK::get_constraint_count() const {
	return constraint_count;
}

void SkeletonModification3DEWBIK::set_kusudama_twist(int32_t p_index, float p_twist) {
	ERR_FAIL_INDEX(p_index, kusudana_twist.size());
	kusudana_twist.write[p_index] = p_twist;
	notify_property_list_changed();
	is_dirty = true;
}

float SkeletonModification3DEWBIK::get_kusudama_twist(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, kusudana_twist.size(), 0.0f);
	return kusudana_twist[p_index];
}

void SkeletonModification3DEWBIK::set_kusudama_limit_cone(int32_t p_bone, int32_t p_index,
		Vector3 p_center, float p_radius) {
	ERR_FAIL_INDEX(p_bone, kusudama_limit_cones.size());
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_bone].size());
	Vector3 center = p_center;
	Color cone;
	cone.r = center.x;
	cone.g = center.y;
	cone.b = center.z;
	cone.a = p_radius;
	kusudama_limit_cones.write[p_bone].write[p_index] = cone;
	// Do not trigger notify_property_list_changed update.
	// Do not trigger the skeleton is dirty.
	// There's a pitfall where if the vector3 is forced normalized
	// here the x component would also change the y component before the edit is finished. 
}

Vector3 SkeletonModification3DEWBIK::get_kusudama_limit_cone_center(int32_t p_bone, int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_bone, kusudama_limit_cones.size(), Vector3(0.0f, 1.0f, 0.0f));
	ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_bone].size(), Vector3(0.0, 0.0, 0.0));
	Color cone = kusudama_limit_cones[p_bone][p_index];
	Vector3 ret;
	ret.x = cone.r;
	ret.y = cone.g;
	ret.z = cone.b;
	return ret;
}

float SkeletonModification3DEWBIK::get_kusudama_limit_cone_radius(int32_t p_bone, int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_bone, kusudama_limit_cones.size(), 0.0f);
	ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_bone].size(), 0.0f);
	return kusudama_limit_cones[p_bone][p_index].a;
}

int32_t SkeletonModification3DEWBIK::get_kusudama_limit_cone_count(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, kusudama_limit_cones.size(), 0);
	return kusudama_limit_cones[p_bone].size();
}

void SkeletonModification3DEWBIK::set_kusudama_limit_cone_count(int32_t p_bone, int32_t p_count) {
	ERR_FAIL_INDEX(p_bone, kusudama_limit_cone_count.size());
	ERR_FAIL_INDEX(p_bone, kusudama_limit_cones.size());
	int32_t old_count = get_kusudama_limit_cone_count(p_bone);
	kusudama_limit_cone_count.write[p_bone] = p_count;
	kusudama_limit_cones.write[p_bone].resize(p_count);
	for (int32_t cone_i = p_count; cone_i-- > old_count;) {
		set_kusudama_limit_cone(p_bone, cone_i, Vector3(0.f, 0.f, 1.0f), 1.0f);
	}
	notify_property_list_changed();
	is_dirty = true;
}

real_t SkeletonModification3DEWBIK::get_default_damp() const {
	return default_damp;
}

void SkeletonModification3DEWBIK::set_default_damp(float p_default_damp) {
	default_damp = p_default_damp;
}

String SkeletonModification3DEWBIK::get_pin_bone_name(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), "");
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_name();
}

void SkeletonModification3DEWBIK::check_shadow_bones_transform() {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		bone->set_initial_pose(skeleton);
		if (bone->is_pinned()) {
			bone->get_pin()->update_goal_global_pose(skeleton);
		}
	}
}
