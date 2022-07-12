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

#include "ewbik.h"
#include "ik_bone_3d.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#endif

int32_t EWBIK::get_ik_iterations() const {
	return ik_iterations;
}

StringName EWBIK::get_root_bone() const {
	return root_bone;
}

void EWBIK::set_root_bone(const StringName &p_root_bone) {
	root_bone = p_root_bone;
	if (skeleton) {
		root_bone_index = skeleton->find_bone(root_bone);
	}
	notify_property_list_changed();
	is_dirty = true;
}

BoneId EWBIK::get_root_bone_index() const {
	return root_bone_index;
}

void EWBIK::set_root_bone_index(BoneId p_index) {
	root_bone_index = p_index;
	if (skeleton) {
		root_bone = skeleton->get_bone_name(p_index);
	}
	notify_property_list_changed();
	is_dirty = true;
}

void EWBIK::set_pin_count(int32_t p_value) {
	int32_t old_count = pins.size();
	pin_count = p_value;
	pins.resize(p_value);
	for (int32_t pin_i = p_value; pin_i-- > old_count;) {
		pins.write[pin_i].instantiate();
	}
	notify_property_list_changed();
	is_dirty = true;
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
	is_dirty = true;
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
	is_dirty = true;
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
	is_dirty = true;
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
	is_dirty = true;
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
	is_dirty = true;
}

void EWBIK::_execute(real_t delta) {
	if (!live_preview) {
		is_dirty = true;
		return;
	}
	if (is_dirty) {
		update_skeleton();
		if (get_debug_skeleton()) {
			set_debug_skeleton(false);
		}
	}
	if (!skeleton) {
		return;
	}
	if (segmented_skeleton.is_null()) {
		return;
	}
	if (bone_list.size()) {
		Ref<IKTransform3D> root_ik_bone = bone_list.write[0]->get_ik_transform();
		ERR_FAIL_NULL(root_ik_bone);
		Ref<IKTransform3D> root_ik_parent_transform = root_ik_bone->get_parent();
		ERR_FAIL_NULL(root_ik_parent_transform);
		root_ik_parent_transform->set_global_transform(skeleton->get_global_transform());
	}
	update_shadow_bones_transform();
	double time_ms = OS::get_singleton()->get_ticks_msec() + get_time_budget_millisecond();
	ik_iterations = 0;
	do {
		segmented_skeleton->segment_solver(get_default_damp());
		ik_iterations++;
	} while (time_ms > OS::get_singleton()->get_ticks_msec() && ik_iterations < get_max_ik_iterations());
	update_skeleton_bones_transform();
}

void EWBIK::_setup_modification(SkeletonModificationStack3D *p_stack) {
}

void EWBIK::update_skeleton() {
	skeleton = cast_to<Skeleton3D>(get_node_or_null(get_skeleton()));
	if (!skeleton) {
		return;
	}
	if (!root_bone) {
		Vector<int32_t> roots = skeleton->get_parentless_bones();
		if (roots.size()) {
			set_root_bone_index(roots[0]);
		}
	}
	ERR_FAIL_COND(root_bone_index == -1);
	ERR_FAIL_COND(!root_bone);
	segmented_skeleton = Ref<IKBoneSegment>(memnew(IKBoneSegment(skeleton, skeleton->get_bone_name(root_bone_index), pins)));
	segmented_skeleton->get_root()->get_ik_transform()->set_parent(root_transform);
	segmented_skeleton->generate_default_segments_from_root(pins);
	bone_list.clear();
	segmented_skeleton->create_bone_list(bone_list, true, debug_skeleton);
	segmented_skeleton->update_pinned_list();
	for (int constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		String bone = constraint_names[constraint_i];
		BoneId bone_id = skeleton->find_bone(bone);
		for (Ref<IKBone3D> ik_bone_3d : bone_list) {
			if (ik_bone_3d->get_bone_id() != bone_id) {
				continue;
			}
			Ref<IKTransform3D> bone_direction_transform;
			bone_direction_transform.instantiate();
			bone_direction_transform->set_parent(ik_bone_3d->get_ik_transform());
			bone_direction_transform->set_transform(Transform3D(Basis(), ik_bone_3d->get_bone_direction_transform()->get_transform().origin));
			Ref<IKKusudama> constraint;
			constraint.instantiate();
			constraint->enable_axial_limits();
			const double axial_from = get_kusudama_twist_from(constraint_i);
			const double axial_to = get_kusudama_twist_to(constraint_i);
			constraint->set_axial_limits(axial_from, axial_to);
			if (get_kusudama_limit_cone_count(constraint_i)) {
				constraint->enable_orientational_limits();
			}
			for (int32_t cone_i = 0; cone_i < get_kusudama_limit_cone_count(constraint_i); cone_i++) {
				constraint->add_limit_cone(get_kusudama_limit_cone_center(constraint_i, cone_i), get_kusudama_limit_cone_radius(constraint_i, cone_i));
			}
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
	is_dirty = false;
}

void EWBIK::update_shadow_bones_transform() {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		bone->set_initial_pose(skeleton);
		if (bone->is_pinned()) {
			bone->get_pin()->update_target_global_transform(skeleton, this);
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
		bone->set_skeleton_bone_pose(skeleton, 1.0);
	}
}

void EWBIK::_validate_property(PropertyInfo &property) const {
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

void EWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "constraint_count", PROPERTY_HINT_RANGE, "0,1024,1,or_greater", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY, "Constraints,constraints/"));
	RBSet<String> existing_constraints;
	for (int32_t constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		const String name = get_constraint_name(constraint_i);
		existing_constraints.insert(name);
	}
	for (int constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		PropertyInfo bone_name;
		bone_name.type = Variant::STRING_NAME;
		bone_name.name = "constraints/" + itos(constraint_i) + "/name";
		if (skeleton) {
			String names;
			for (int bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
				String name = skeleton->get_bone_name(bone_i);
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
						vformat("Limit Cones,constraints/%s/kusudama_limit_cone/", itos(constraint_i))));
		for (int cone_i = 0; cone_i < get_kusudama_limit_cone_count(constraint_i); cone_i++) {
			p_list->push_back(
					PropertyInfo(Variant::VECTOR3, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/center", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"));
			p_list->push_back(
					PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/radius", PROPERTY_HINT_RANGE, "0,360,0.1,radians,exp"));
		}
	}
	p_list->push_back(PropertyInfo(Variant::INT, "pin_count", PROPERTY_HINT_RANGE, "0,1024,1", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY, "Pins,pins/"));
	for (int pin_i = 0; pin_i < pin_count; pin_i++) {
		PropertyInfo effector_name;
		effector_name.type = Variant::STRING_NAME;
		effector_name.name = "pins/" + itos(pin_i) + "/name";
		if (skeleton) {
			RBSet<String> existing_pins;
			for (Ref<IKEffectorTemplate> pin : pins) {
				if (pin.is_null()) {
					continue;
				}
				const String name = pin->get_name();
				existing_pins.insert(name);
			}
			String names;
			for (int bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
				String name = skeleton->get_bone_name(bone_i);
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
	} else if (name.begins_with("constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		if (index >= constraint_count) {
			return false;
		}
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
			if (cone_index >= kusudama_limit_cone_count[index]) {
				return false;
			}
			if (cone_what == "center") {
				Vector3 center = p_value;
				if (Math::is_zero_approx(center.length_squared())) {
					center = Vector3(0.0, 1.0, 0.0);
				} else {
					center.normalize();
				}
				set_kusudama_limit_cone_center(index, cone_index, center);
				return true;
			} else if (cone_what == "radius") {
				set_kusudama_limit_cone_radius(index, cone_index, p_value);
				return true;
			}
		}
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
	}
	return false;
}

void EWBIK::_bind_methods() {
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
	ClassDB::bind_method(D_METHOD("get_time_budget_millisecond"), &EWBIK::get_time_budget_millisecond);
	ClassDB::bind_method(D_METHOD("set_time_budget_millisecond", "budget"), &EWBIK::set_time_budget_millisecond);
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &EWBIK::get_ik_iterations);
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
	ClassDB::bind_method(D_METHOD("update_skeleton"), &EWBIK::update_skeleton);
	ClassDB::bind_method(D_METHOD("get_debug_skeleton"), &EWBIK::get_debug_skeleton);
	ClassDB::bind_method(D_METHOD("set_debug_skeleton", "enabled"), &EWBIK::set_debug_skeleton);
	ClassDB::bind_method(D_METHOD("get_default_damp"), &EWBIK::get_default_damp);
	ClassDB::bind_method(D_METHOD("set_default_damp", "damp"), &EWBIK::set_default_damp);
	ClassDB::bind_method(D_METHOD("get_kusudama_flip_handedness"), &EWBIK::get_kusudama_flip_handedness);
	ClassDB::bind_method(D_METHOD("set_live_preview", "enable"), &EWBIK::set_live_preview);
	ClassDB::bind_method(D_METHOD("get_live_preview"), &EWBIK::get_live_preview);
	ClassDB::bind_method(D_METHOD("set_skeleton", "skeleton"), &EWBIK::set_skeleton);
	ClassDB::bind_method(D_METHOD("get_skeleton"), &EWBIK::get_skeleton);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "iterations", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "", "get_ik_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_ik_iterations", PROPERTY_HINT_RANGE, "1,150,1,or_greater"), "set_max_ik_iterations", "get_max_ik_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "budget_millisecond", PROPERTY_HINT_RANGE, "0.01,2.0,0.01,or_greater"), "set_time_budget_millisecond", "get_time_budget_millisecond");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "default_damp", PROPERTY_HINT_RANGE, "0.04,179.99,0.01,radians,exp", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_default_damp", "get_default_damp");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "print_skeleton", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_debug_skeleton", "get_debug_skeleton");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live_preview"), "set_live_preview", "get_live_preview");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "skeleton", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_skeleton", "get_skeleton");
}

EWBIK::EWBIK() {
}

EWBIK::~EWBIK() {
}

bool EWBIK::get_debug_skeleton() const {
	return debug_skeleton;
}

void EWBIK::set_debug_skeleton(bool p_enabled) {
	debug_skeleton = p_enabled;
	if (skeleton) {
		skeleton->notify_property_list_changed();
	}
	is_dirty = true;
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
	is_dirty = true;
}

void EWBIK::set_constraint_count(int32_t p_count) {
	int32_t old_count = constraint_names.size();
	constraint_count = p_count;
	constraint_names.resize(p_count);
	kusudama_twist_from.resize(p_count);
	kusudama_twist_to.resize(p_count);
	kusudama_flip_handedness.resize(p_count);
	for (int32_t constraint_i = p_count; constraint_i-- > old_count;) {
		constraint_names.write[constraint_i] = String();
		kusudama_twist_from.write[constraint_i] = 0.0f;
		kusudama_twist_to.write[constraint_i] = 0.0f;
		kusudama_flip_handedness.write[constraint_i] = false;
	}
	notify_property_list_changed();
	is_dirty = true;
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
	notify_property_list_changed();
	is_dirty = true;
}

void EWBIK::set_kusudama_twist_to(int32_t p_index, float p_to) {
	ERR_FAIL_INDEX(p_index, constraint_count);
	kusudama_twist_to.write[p_index] = p_to;
	notify_property_list_changed();
	is_dirty = true;
}

int32_t EWBIK::find_effector_id(StringName p_bone_name) {
	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		if (constraint_names[constraint_i] == p_bone_name) {
			return constraint_i;
		}
	}
	return -1;
}

void EWBIK::set_kusudama_limit_cone(int32_t p_constraint, int32_t p_index,
		Vector3 p_center, float p_radius) {
	PackedColorArray cones;
	if (kusudama_limit_cones.has(p_constraint)) {
		cones = kusudama_limit_cones[p_constraint];
	}
	Vector3 center = p_center;
	Color cone;
	cone.r = center.x;
	cone.g = center.y;
	cone.b = center.z;
	cone.a = p_radius;
	cones.write[p_index] = cone;
	kusudama_limit_cones[p_constraint] = cones;
	// Must notify the skeleton too.
	if (skeleton) {
		skeleton->notify_property_list_changed();
	}
	is_dirty = true;
}

Vector3 EWBIK::get_kusudama_limit_cone_center(int32_t p_bone, int32_t p_index) const {
	ERR_FAIL_COND_V(!kusudama_limit_cones.has(p_bone), Vector3(0.0, 1.0, 0.0));
	ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_bone].size(), Vector3(0.0, 1.0, 0.0));
	const Color &cone = kusudama_limit_cones[p_bone][p_index];
	Vector3 ret;
	ret.x = cone.r;
	ret.y = cone.g;
	ret.z = cone.b;
	return ret;
}

float EWBIK::get_kusudama_limit_cone_radius(int32_t p_bone, int32_t p_index) const {
	ERR_FAIL_COND_V(!kusudama_limit_cones.has(p_bone), Math_TAU);
	ERR_FAIL_INDEX_V(p_index, kusudama_limit_cones[p_bone].size(), Math_TAU);
	return kusudama_limit_cones[p_bone][p_index].a;
}

int32_t EWBIK::get_kusudama_limit_cone_count(int32_t p_effector) const {
	if (!kusudama_limit_cone_count.has(p_effector)) {
		return 0;
	}
	return kusudama_limit_cone_count[p_effector];
}

void EWBIK::set_kusudama_limit_cone_count(int32_t p_effector, int32_t p_count) {
	int32_t old_count = get_kusudama_limit_cone_count(p_effector);
	kusudama_limit_cone_count[p_effector] = p_count;
	PackedColorArray &cones = kusudama_limit_cones[p_effector];
	cones.resize(p_count);
	for (int32_t cone_i = p_count; cone_i-- > old_count;) {
		Color &cone = cones.write[cone_i];
		cone.r = 0.0f;
		cone.g = 1.0f;
		cone.b = 0.0f;
		cone.a = 0.0f;
	}
	// Must notify the skeleton too.
	if (skeleton) {
		skeleton->notify_property_list_changed();
	}
	is_dirty = true;
}

real_t EWBIK::get_default_damp() const {
	return default_damp;
}

void EWBIK::set_default_damp(float p_default_damp) {
	default_damp = p_default_damp;
	// Must notify the skeleton too.
	if (skeleton) {
		skeleton->notify_property_list_changed();
	}
	is_dirty = true;
}

StringName EWBIK::get_pin_bone_name(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), "");
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_name();
}

void EWBIK::set_kusudama_limit_cone_radius(int32_t p_effector_index, int32_t p_index, float p_radius) {
	ERR_FAIL_COND(!kusudama_limit_cones.has(p_effector_index));
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_effector_index].size());
	PackedColorArray &cones = kusudama_limit_cones[p_effector_index];
	Color &cone = cones.write[p_index];
	cone.a = p_radius;

	// Must notify the skeleton too.
	if (skeleton) {
		skeleton->notify_property_list_changed();
	}
	is_dirty = true;
}

void EWBIK::set_kusudama_limit_cone_center(int32_t p_effector_index, int32_t p_index, Vector3 p_center) {
	ERR_FAIL_COND(!kusudama_limit_cones.has(p_effector_index));
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_effector_index].size());
	PackedColorArray &cones = kusudama_limit_cones[p_effector_index];
	Color &cone = cones.write[p_index];
	cone.r = p_center.x;
	cone.g = p_center.y;
	cone.b = p_center.z;

	// Must notify the skeleton too.
	if (skeleton) {
		skeleton->notify_property_list_changed();
	}
	is_dirty = true;
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
	is_dirty = true;
}

void EWBIK::set_dirty() {
	is_dirty = true;
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

float EWBIK::get_time_budget_millisecond() const {
	return time_budget_millisecond;
}

void EWBIK::set_time_budget_millisecond(const float &p_time_budget) {
	time_budget_millisecond = p_time_budget;
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
	set_dirty();
}

void EWBIK::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_setup_modification(nullptr);
			set_process_internal(false);
			set_physics_process_internal(true);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (!Engine::get_singleton()->is_editor_hint() || live_preview) {
				_execute(get_process_delta_time());
			}
		} break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			if (!Engine::get_singleton()->is_editor_hint() || live_preview) {
				_execute(get_physics_process_delta_time());
			}
		} break;
	}
}