/*************************************************************************/
/*  ewbik.cpp                                                            */
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

#include "ewbik.h"
#include "core/core_string_names.h"
#include "ik_bone_3d.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#endif

void EWBIK::set_pin_count(int32_t p_value) {
	int32_t old_count = pins.size();
	pin_count = p_value;
	pins.resize(p_value);
	for (int32_t pin_i = p_value; pin_i-- > old_count;) {
		pins.write[pin_i].instantiate();
	}
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

int32_t EWBIK::get_pin_count() const {
	return pin_count;
}

void EWBIK::add_pin(const StringName &p_name, const NodePath &p_target_node, const bool &p_use_node_rotation) {
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
	skeleton_changed(get_skeleton());
}

void EWBIK::set_pin_bone(int32_t p_pin_index, const String &p_bone) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_pin_index];
	if (data.is_null()) {
		data.instantiate();
		pins.write[p_pin_index] = data;
	}
	data->set_name(p_bone);
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

void EWBIK::set_pin_target_nodepath(int32_t p_pin_index, const NodePath &p_target_node) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_pin_index];
	if (data.is_null()) {
		data.instantiate();
		pins.write[p_pin_index] = data;
	}
	data->set_target_node(p_target_node);
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

NodePath EWBIK::get_pin_target_nodepath(int32_t p_pin_index) {
	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), NodePath());
	const Ref<IKEffectorTemplate> data = pins[p_pin_index];
	return data->get_target_node();
}

void EWBIK::set_pin_use_node_rotation(int32_t p_pin_index, bool p_use_node_rot) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_pin_index];
	ERR_FAIL_NULL(data);
	data->set_target_node_rotation(p_use_node_rot);

	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

bool EWBIK::get_pin_use_node_rotation(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), false);
	const Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_target_node_rotation();
}

Vector<Ref<IKEffectorTemplate>> EWBIK::get_bone_effectors() const {
	return pins;
}

void EWBIK::remove_pin(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, pins.size());
	pins.remove_at(p_index);
	pin_count--;
	pins.resize(pin_count);
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

void EWBIK::update_shadow_bones_transform() {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		bone->set_initial_pose(get_skeleton());
		if (bone->is_pinned()) {
			bone->get_pin()->update_target_global_transform(get_skeleton(), this);
		}
	}
}

void EWBIK::update_skeleton_bones_transform() {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		if (bone->get_bone_id() == -1) {
			continue;
		}
		bone->set_skeleton_bone_pose(get_skeleton(), 1.0);
	}
}

void EWBIK::_validate_property(PropertyInfo &property) const {
	if (property.name == "root_bone") {
		if (get_skeleton()) {
			String names;
			for (int i = 0; i < get_skeleton()->get_bone_count(); i++) {
				String name = get_skeleton()->get_bone_name(i);
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
	if (property.name == "tip_bone") {
		if (get_skeleton()) {
			String names;
			BoneId root_bone_id = get_skeleton()->find_bone(root_bone);
			for (int i = 0; i < get_skeleton()->get_bone_count(); i++) {
				if (i <= root_bone_id && root_bone_id != -1) {
					continue;
				}
				String name = get_skeleton()->get_bone_name(i);
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

void EWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	RBSet<String> existing_pins;
	for (int32_t pin_i = 0; pin_i < get_pin_count(); pin_i++) {
		const String name = get_pin_bone_name(pin_i);
		existing_pins.insert(name);
	}
	p_list->push_back(
			PropertyInfo(Variant::INT, "pin_count",
					PROPERTY_HINT_RANGE, "0,1024,or_greater", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY,
					"Pins,pins/"));
	for (int pin_i = 0; pin_i < pin_count; pin_i++) {
		PropertyInfo effector_name;
		effector_name.type = Variant::STRING_NAME;
		effector_name.name = "pins/" + itos(pin_i) + "/name";
		if (get_skeleton()) {
			String names;
			for (int bone_i = 0; bone_i < get_skeleton()->get_bone_count(); bone_i++) {
				String name = get_skeleton()->get_bone_name(bone_i);
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
				PropertyInfo(Variant::NODE_PATH, "pins/" + itos(pin_i) + "/target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"));

		p_list->push_back(PropertyInfo(Variant::BOOL, "pins/" + itos(pin_i) + "/use_node_rotation"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/depth_falloff", PROPERTY_HINT_RANGE, "0,1,0.01"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/priority", PROPERTY_HINT_RANGE, "0,1,0.01"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/weight_translation", PROPERTY_HINT_RANGE, "0,1,0.01"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/weight_x_direction", PROPERTY_HINT_RANGE, "0,1,0.01"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/weight_y_direction", PROPERTY_HINT_RANGE, "0,1,0.01"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/weight_z_direction", PROPERTY_HINT_RANGE, "0,1,0.01"));
	}
	RBSet<String> existing_constraints;
	for (int32_t constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		const String name = get_constraint_name(constraint_i);
		existing_constraints.insert(name);
	}
	p_list->push_back(
			PropertyInfo(Variant::INT, "constraint_count",
					PROPERTY_HINT_RANGE, "0,1024,or_greater", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY,
					"Constraints,constraints/"));
	for (int constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		PropertyInfo bone_name;
		bone_name.type = Variant::STRING_NAME;
		bone_name.name = "constraints/" + itos(constraint_i) + "/name";
		if (get_skeleton()) {
			String names;
			for (int bone_i = 0; bone_i < get_skeleton()->get_bone_count(); bone_i++) {
				String name = get_skeleton()->get_bone_name(bone_i);
				if (existing_constraints.has(name)) {
					continue;
				}
				name += ",";
				names += name;
				existing_constraints.insert(name);
			}
			bone_name.hint = PROPERTY_HINT_ENUM_SUGGESTION;
			bone_name.hint_string = names;
		} else {
			bone_name.hint = PROPERTY_HINT_NONE;
			bone_name.hint_string = "";
		}
		p_list->push_back(bone_name);
		p_list->push_back(
				PropertyInfo(Variant::BOOL, "constraints/" + itos(constraint_i) + "/kusudama_flip_handedness"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_twist_from", PROPERTY_HINT_RANGE, "-360.0,360.0,0.1,radians,exp"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_twist_to", PROPERTY_HINT_RANGE, "-360.0,360.0,0.1,radians,exp"));
		p_list->push_back(
				PropertyInfo(Variant::INT, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone_count",
						PROPERTY_HINT_RANGE, "0,30,1", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY,
						"Limit Cones,constraints/" + itos(constraint_i) + "/kusudama_limit_cone/"));
		for (int cone_i = 0; cone_i < get_kusudama_limit_cone_count(constraint_i); cone_i++) {
			p_list->push_back(
					PropertyInfo(Variant::VECTOR3, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/center", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"));
			p_list->push_back(
					PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/radius", PROPERTY_HINT_RANGE, "0,360,0.1,radians,exp"));
		}
	}
}

bool EWBIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "constraint_count") {
		r_ret = get_constraint_count();
		return true;
	} else if (name == "pin_count") {
		r_ret = get_pin_count();
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
		} else if (what == "priority") {
			return true;
		} else if (what == "weight_translation") {
			return true;
		} else if (what == "weight_x_direction") {
			return true;
		} else if (what == "weight_y_direction") {
			return true;
		} else if (what == "weight_z_direction") {
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
		} else if (what == "kusudama_flip_handedness") {
			r_ret = get_kusudama_flip_handedness(index);
			return true;
		} else if (what == "kusudama_twist_from") {
			r_ret = get_kusudama_twist_from(index);
			return true;
		} else if (what == "kusudama_twist_to") {
			r_ret = get_kusudama_twist_to(index);
			return true;
		} else if (what == "kusudama_limit_cone_count") {
			r_ret = get_kusudama_limit_cone_count(index);
			return true;
		} else if (name.begins_with(begins)) {
			int32_t cone_index = name.get_slicec('/', 3).to_int();
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

bool EWBIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "constraint_count") {
		set_constraint_count(p_value);
		return true;
	} else if (name == "pin_count") {
		set_pin_count(p_value);
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
			return true;
		} else if (what == "use_node_rotation") {
			set_pin_use_node_rotation(index, p_value);
			return true;
		} else if (what == "depth_falloff") {
			set_pin_depth_falloff(index, p_value);
			return true;
		} else if (what == "priority") {
			return true;
		} else if (what == "weight_translation") {
			return true;
		} else if (what == "weight_x_direction") {
			return true;
		} else if (what == "weight_y_direction") {
			return true;
		} else if (what == "weight_z_direction") {
			return true;
		}
	} else if (name.begins_with("constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		String begins = "constraints/" + itos(index) + "/kusudama_limit_cone/";
		if (what == "name") {
			if (index >= constraint_names.size()) {
				set_constraint_count(constraint_count);
			}
			set_constraint_name(index, p_value);
			return true;
		} else if (what == "kusudama_flip_handedness") {
			set_kusudama_flip_handedness(index, p_value);
			return true;
		} else if (what == "kusudama_twist_from") {
			set_kusudama_twist_from(index, p_value);
			return true;
		} else if (what == "kusudama_twist_to") {
			set_kusudama_twist_to(index, p_value);
			return true;
		} else if (what == "kusudama_limit_cone_count") {
			set_kusudama_limit_cone_count(index, p_value);
			return true;
		} else if (name.begins_with(begins)) {
			int cone_index = name.get_slicec('/', 3).to_int();
			String cone_what = name.get_slicec('/', 4);
			if (cone_what == "center") {
				Vector3 center = p_value;
				if (Math::is_zero_approx(center.length_squared())) {
					center = Vector3(0.0, 1.0, 0.0);
				}
				set_kusudama_limit_cone_center(index, cone_index, center);
				return true;
			} else if (cone_what == "radius") {
				set_kusudama_limit_cone_radius(index, cone_index, p_value);
				return true;
			}
		}
	}
	return false;
}

void EWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_root_bone", "root_bone"), &EWBIK::set_root_bone);
	ClassDB::bind_method(D_METHOD("get_root_bone"), &EWBIK::get_root_bone);
	ClassDB::bind_method(D_METHOD("set_tip_bone", "tip_bone"), &EWBIK::set_tip_bone);
	ClassDB::bind_method(D_METHOD("get_tip_bone"), &EWBIK::get_tip_bone);
	ClassDB::bind_method(D_METHOD("set_kusudama_limit_cone_radius", "index", "cone_index", "radius"), &EWBIK::set_kusudama_limit_cone_radius);
	ClassDB::bind_method(D_METHOD("get_kusudama_limit_cone_radius", "index", "cone_index"), &EWBIK::get_kusudama_limit_cone_radius);
	ClassDB::bind_method(D_METHOD("set_kusudama_limit_cone_center", "index", "cone_index", "center"), &EWBIK::set_kusudama_limit_cone_center);
	ClassDB::bind_method(D_METHOD("get_kusudama_limit_cone_center", "index", "cone_index"), &EWBIK::set_kusudama_limit_cone_center);
	ClassDB::bind_method(D_METHOD("set_kusudama_limit_cone_count", "index", "count"), &EWBIK::set_kusudama_limit_cone_count);
	ClassDB::bind_method(D_METHOD("get_kusudama_limit_cone_count", "index"), &EWBIK::get_kusudama_limit_cone_count);
	ClassDB::bind_method(D_METHOD("set_kusudama_twist_from", "index", "name"), &EWBIK::set_kusudama_twist_from);
	ClassDB::bind_method(D_METHOD("get_kusudama_twist_from", "index"), &EWBIK::get_kusudama_twist_from);
	ClassDB::bind_method(D_METHOD("set_kusudama_twist_to", "index", "name"), &EWBIK::set_kusudama_twist_to);
	ClassDB::bind_method(D_METHOD("get_kusudama_twist_to", "index"), &EWBIK::get_kusudama_twist_to);
	ClassDB::bind_method(D_METHOD("set_pin_depth_falloff", "index", "falloff"), &EWBIK::set_pin_depth_falloff);
	ClassDB::bind_method(D_METHOD("get_pin_depth_falloff", "index"), &EWBIK::get_pin_depth_falloff);
	ClassDB::bind_method(D_METHOD("set_constraint_name", "index", "name"), &EWBIK::set_constraint_name);
	ClassDB::bind_method(D_METHOD("get_constraint_name", "index"), &EWBIK::get_constraint_name);
	ClassDB::bind_method(D_METHOD("get_segmented_skeleton"), &EWBIK::get_segmented_skeleton);
	ClassDB::bind_method(D_METHOD("get_max_ik_iterations"), &EWBIK::get_max_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_max_ik_iterations", "count"), &EWBIK::set_max_ik_iterations);
	ClassDB::bind_method(D_METHOD("get_constraint_count"), &EWBIK::get_constraint_count);
	ClassDB::bind_method(D_METHOD("set_constraint_count", "count"),
			&EWBIK::set_constraint_count);
	ClassDB::bind_method(D_METHOD("get_pin_count"), &EWBIK::get_pin_count);
	ClassDB::bind_method(D_METHOD("set_pin_count", "count"),
			&EWBIK::set_pin_count);
	ClassDB::bind_method(D_METHOD("remove_pin", "index"),
			&EWBIK::remove_pin);
	ClassDB::bind_method(D_METHOD("add_pin", "name", "target_node", "use_node_rotation"), &EWBIK::add_pin);
	ClassDB::bind_method(D_METHOD("get_pin_bone_name", "index"), &EWBIK::get_pin_bone_name);
	ClassDB::bind_method(D_METHOD("set_pin_bone_name", "index", "name"), &EWBIK::set_pin_bone_name);
	ClassDB::bind_method(D_METHOD("print_debug_skeleton"), &EWBIK::print_debug_skeleton);
	ClassDB::bind_method(D_METHOD("get_default_damp"), &EWBIK::get_default_damp);
	ClassDB::bind_method(D_METHOD("set_default_damp", "damp"), &EWBIK::set_default_damp);
	ClassDB::bind_method(D_METHOD("get_kusudama_flip_handedness"), &EWBIK::get_kusudama_flip_handedness);
	ClassDB::bind_method(D_METHOD("get_pin_nodepath"), &EWBIK::get_pin_nodepath);
	ClassDB::bind_method(D_METHOD("set_pin_nodepath", "index", "nodepath"), &EWBIK::set_pin_nodepath);
	ClassDB::bind_method(D_METHOD("set_pin_use_node_rotation", "index", "node_rotation"), &EWBIK::set_pin_use_node_rotation);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "root_bone", PROPERTY_HINT_ENUM_SUGGESTION), "set_root_bone", "get_root_bone");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "tip_bone", PROPERTY_HINT_ENUM_SUGGESTION), "set_tip_bone", "get_tip_bone");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_ik_iterations", PROPERTY_HINT_RANGE, "1,150,1,or_greater"), "set_max_ik_iterations", "get_max_ik_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "default_damp", PROPERTY_HINT_RANGE, "0.04,179.99,0.01,radians,exp", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_default_damp", "get_default_damp");
}

EWBIK::EWBIK() {
}

EWBIK::~EWBIK() {
}


void EWBIK::print_debug_skeleton() {
	debug_skeleton = true;
	if (get_skeleton()) {
		get_skeleton()->notify_property_list_changed();
	}
	skeleton_changed(get_skeleton());
	debug_skeleton = false;
}

float EWBIK::get_pin_depth_falloff(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), 0.0f);
	const Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_depth_falloff();
}

void EWBIK::set_pin_depth_falloff(int32_t p_effector_index, const float p_depth_falloff) {
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	ERR_FAIL_NULL(data);
	data->set_depth_falloff(p_depth_falloff);
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

void EWBIK::set_constraint_count(int32_t p_count) {
	int32_t old_count = constraint_names.size();
	constraint_count = p_count;
	constraint_names.resize(p_count);
	kusudama_twist_from.resize(p_count);
	kusudama_twist_to.resize(p_count);
	kusudama_flip_handedness.resize(p_count);
	kusudama_limit_cone_count.resize(p_count);
	kusudama_limit_cones.resize(p_count);
	for (int32_t constraint_i = p_count; constraint_i-- > old_count;) {
		constraint_names.write[constraint_i] = String();
		kusudama_twist_from.write[constraint_i] = -(Math_TAU - CMP_EPSILON);
		kusudama_twist_to.write[constraint_i] = Math_TAU - CMP_EPSILON;
		kusudama_flip_handedness.write[constraint_i] = false;
		kusudama_limit_cone_count.write[constraint_i] = 0;
		kusudama_limit_cones.write[constraint_i].resize(0);
	}

	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

int32_t EWBIK::get_constraint_count() const {
	return constraint_count;
}

inline StringName EWBIK::get_constraint_name(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), StringName());
	return constraint_names[p_index];
}

void EWBIK::set_kusudama_twist_from(int32_t p_index, float p_from) {
	ERR_FAIL_INDEX(p_index, constraint_count);
	kusudama_twist_from.write[p_index] = p_from;
	skeleton_changed(get_skeleton());
}

void EWBIK::set_kusudama_twist_to(int32_t p_index, float p_to) {
	ERR_FAIL_INDEX(p_index, constraint_count);
	kusudama_twist_to.write[p_index] = p_to;
	skeleton_changed(get_skeleton());
}

int32_t EWBIK::find_effector_id(StringName p_bone_name) {
	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		if (constraint_names[constraint_i] == p_bone_name) {
			return constraint_i;
		}
	}
	return -1;
}

void EWBIK::set_kusudama_limit_cone(int32_t p_contraint_index, int32_t p_index,
		Vector3 p_center, float p_radius) {
	ERR_FAIL_INDEX(p_contraint_index, kusudama_limit_cones.size());
	Vector<Vector4> cones = kusudama_limit_cones.write[p_contraint_index];
	Vector3 center = p_center;
	Vector4 cone;
	cone.x = center.x;
	cone.y = center.y;
	cone.z = center.z;
	cone.w = p_radius;
	cones.write[p_index] = cone;
	kusudama_limit_cones.write[p_contraint_index] = cones;
	skeleton_changed(get_skeleton());
}

Vector3 EWBIK::get_kusudama_limit_cone_center(int32_t p_contraint_index, int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_contraint_index, kusudama_limit_cone_count.size(), Vector3(0.0, 1.0, 0.0));
	ERR_FAIL_INDEX_V(p_contraint_index, kusudama_limit_cones.size(), Vector3(0.0, 1.0, 0.0));
	ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_contraint_index].size(), Vector3(0.0, 1.0, 0.0));
	const Vector4 &cone = kusudama_limit_cones[p_contraint_index][p_index];
	Vector3 ret;
	ret.x = cone.x;
	ret.y = cone.y;
	ret.z = cone.z;
	return ret;
}

float EWBIK::get_kusudama_limit_cone_radius(int32_t p_contraint_index, int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_contraint_index, kusudama_limit_cone_count.size(), Math_TAU);
	ERR_FAIL_INDEX_V(p_contraint_index, kusudama_limit_cones.size(), Math_TAU);
	ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_contraint_index].size(), Math_TAU);
	return kusudama_limit_cones[p_contraint_index][p_index].w;
}

int32_t EWBIK::get_kusudama_limit_cone_count(int32_t p_contraint_index) const {
	return kusudama_limit_cone_count[p_contraint_index];
}

void EWBIK::set_kusudama_limit_cone_count(int32_t p_contraint_index, int32_t p_count) {
	ERR_FAIL_INDEX(p_contraint_index, kusudama_limit_cone_count.size());
	ERR_FAIL_INDEX(p_contraint_index, kusudama_limit_cones.size());
	int32_t old_cone_count = kusudama_limit_cones[p_contraint_index].size();
	kusudama_limit_cone_count.write[p_contraint_index] = p_count;
	Vector<Vector4> &cones = kusudama_limit_cones.write[p_contraint_index];
	cones.resize(p_count);
	for (int32_t cone_i = p_count; cone_i-- > old_cone_count;) {
		Vector4 &cone = cones.write[cone_i];
		cone.x = 0.0f;
		cone.y = 1.0f;
		cone.z = 0.0f;
		cone.w = Math::deg_to_rad(10.0f);
	}

	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

real_t EWBIK::get_default_damp() const {
	return default_damp;
}

void EWBIK::set_default_damp(float p_default_damp) {
	default_damp = p_default_damp;
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

StringName EWBIK::get_pin_bone_name(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), "");
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_name();
}

void EWBIK::set_kusudama_limit_cone_radius(int32_t p_effector_index, int32_t p_index, float p_radius) {
	ERR_FAIL_INDEX(p_effector_index, kusudama_limit_cone_count.size());
	ERR_FAIL_INDEX(p_effector_index, kusudama_limit_cones.size());
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_effector_index].size());
	Vector4 &cone = kusudama_limit_cones.write[p_effector_index].write[p_index];
	cone.w = p_radius;
	skeleton_changed(get_skeleton());
}

void EWBIK::set_kusudama_limit_cone_center(int32_t p_effector_index, int32_t p_index, Vector3 p_center) {
	ERR_FAIL_INDEX(p_effector_index, kusudama_limit_cone_count.size());
	ERR_FAIL_INDEX(p_effector_index, kusudama_limit_cones.size());
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_effector_index].size());
	Vector4 &cone = kusudama_limit_cones.write[p_effector_index].write[p_index];
	cone.x = p_center.x;
	cone.y = p_center.y;
	cone.z = p_center.z;
	skeleton_changed(get_skeleton());
}

float EWBIK::get_kusudama_twist_from(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, kusudama_twist_from.size(), 0.0f);
	return kusudama_twist_from[p_index];
}

float EWBIK::get_kusudama_twist_to(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, kusudama_twist_to.size(), 360.0f);
	return kusudama_twist_to[p_index];
}

void EWBIK::set_constraint_name(int32_t p_index, String p_name) {
	ERR_FAIL_INDEX(p_index, constraint_names.size());
	constraint_names.write[p_index] = p_name;
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

Ref<IKBoneSegment> EWBIK::get_segmented_skeleton() {
	return segmented_skeleton;
}
float EWBIK::get_max_ik_iterations() const {
	return max_ik_iterations;
}

void EWBIK::set_max_ik_iterations(const float &p_max_ik_iterations) {
	max_ik_iterations = p_max_ik_iterations;
}

bool EWBIK::get_kusudama_flip_handedness(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, kusudama_flip_handedness.size(), false);
	return kusudama_flip_handedness[p_bone];
}

void EWBIK::set_kusudama_flip_handedness(int32_t p_bone, bool p_flip) {
	ERR_FAIL_INDEX(p_bone, kusudama_flip_handedness.size());
	kusudama_flip_handedness.write[p_bone] = p_flip;
	if (segmented_skeleton.is_null()) {
		return;
	}
	Ref<IKBone3D> bone = segmented_skeleton->get_ik_bone(p_bone);
	if (bone.is_null()) {
		return;
	}
	Ref<IKTransform3D> constraint_transform = bone->get_constraint_transform();
	if (constraint_transform.is_null()) {
		return;
	}
	constraint_transform->set_global_chirality(p_flip ? -1.0 : 1.0);
	Ref<IKTransform3D> transform = bone->get_ik_transform();
	if (transform.is_null()) {
		return;
	}
	transform->set_global_chirality(p_flip ? -1.0 : 1.0);
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

void EWBIK::set_pin_bone_name(int32_t p_effector_index, StringName p_name) const {
	ERR_FAIL_INDEX(p_effector_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	data->set_name(p_name);
}

void EWBIK::set_pin_nodepath(int32_t p_effector_index, NodePath p_node_path) {
	ERR_FAIL_INDEX(p_effector_index, pins.size());
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	data->set_target_node(p_node_path);
}

NodePath EWBIK::get_pin_nodepath(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), NodePath());
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_target_node();
}

void EWBIK::execute(real_t delta) {
	SkeletonModification3D::execute(delta);
	if (segmented_skeleton.is_null()) {
		return;
	}
	if (bone_list.size()) {
		Ref<IKTransform3D> root_ik_bone = bone_list.write[0]->get_ik_transform();
		ERR_FAIL_NULL(root_ik_bone);
		Ref<IKTransform3D> root_ik_parent_transform = root_ik_bone->get_parent();
		ERR_FAIL_NULL(root_ik_parent_transform);
		root_ik_parent_transform->set_global_transform(get_skeleton()->get_global_transform());
	}
	update_shadow_bones_transform();
	for (int32_t i = 0; i < get_max_ik_iterations(); i++) {
		segmented_skeleton->segment_solver(get_default_damp());
	}
	update_skeleton_bones_transform();
}

void EWBIK::skeleton_changed(Skeleton3D *p_skeleton) {
	if (!p_skeleton) {
		return;
	}
	if (!root_bone) {
		Vector<int32_t> roots = p_skeleton->get_parentless_bones();
		if (roots.size()) {
			StringName parentless_bone = p_skeleton->get_bone_name(roots[0]);
			set_root_bone(parentless_bone);
		}
	}
	ERR_FAIL_COND(!root_bone);
	BoneId root_bone_index = p_skeleton->find_bone(root_bone);
	BoneId tip_bone_index = p_skeleton->find_bone(tip_bone);
	segmented_skeleton = Ref<IKBoneSegment>(memnew(IKBoneSegment(p_skeleton, root_bone, pins, nullptr, root_bone_index, tip_bone_index)));
	segmented_skeleton->get_root()->get_ik_transform()->set_parent(root_transform);
	segmented_skeleton->generate_default_segments_from_root(pins, root_bone_index, tip_bone_index);
	bone_list.clear();
	segmented_skeleton->create_bone_list(bone_list, true, debug_skeleton);
	segmented_skeleton->update_pinned_list();
	update_shadow_bones_transform();
	for (int constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		String bone = constraint_names[constraint_i];
		BoneId bone_id = p_skeleton->find_bone(bone);
		for (Ref<IKBone3D> ik_bone_3d : bone_list) {
			if (ik_bone_3d->get_bone_id() != bone_id) {
				continue;
			}
			Ref<IKTransform3D> bone_direction_transform;
			bone_direction_transform.instantiate();
			bone_direction_transform->set_parent(ik_bone_3d->get_ik_transform());
			bone_direction_transform->set_transform(Transform3D(Basis(), ik_bone_3d->get_bone_direction_transform()->get_transform().origin));
			Ref<IKKusudama> constraint = memnew(IKKusudama(ik_bone_3d));
			constraint->enable_axial_limits();
			const double axial_from = get_kusudama_twist_from(constraint_i);
			const double axial_to = get_kusudama_twist_to(constraint_i);
			for (int32_t cone_i = 0; cone_i < kusudama_limit_cone_count[constraint_i]; cone_i++) {
				if (cone_i == 0) {
					constraint->enable_orientational_limits();
				}
				Vector4 cone = kusudama_limit_cones[constraint_i][cone_i];
				constraint->add_limit_cone(Vector3(cone.x, cone.y, cone.z), cone.w);
			}
			constraint->_update_constraint();
			constraint->set_axial_limits(axial_from, axial_to);
			ik_bone_3d->add_constraint(constraint);
			break;
		}
	}
	for (Ref<IKBone3D> ik_bone_3d : bone_list) {
		Ref<IKKusudama> constraint = ik_bone_3d->get_constraint();
		if (constraint.is_null()) {
			continue;
		}
		constraint->update_tangent_radii();
		constraint->update_rotational_freedom();
	}
	SkeletonModification3D::skeleton_changed(p_skeleton);
}

StringName EWBIK::get_root_bone() const {
	return root_bone;
}

void EWBIK::set_root_bone(const StringName &p_root_bone) {
	root_bone = p_root_bone;
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}

StringName EWBIK::get_tip_bone() const {
	return tip_bone;
}

void EWBIK::set_tip_bone(StringName p_bone) {
	tip_bone = p_bone;
	notify_property_list_changed();
	skeleton_changed(get_skeleton());
}
