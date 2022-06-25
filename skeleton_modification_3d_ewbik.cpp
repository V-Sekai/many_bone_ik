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
#include "ik_bone_3d.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#endif

int32_t SkeletonModification3DEWBIK::get_ik_iterations() const {
	return ik_iterations;
}

StringName SkeletonModification3DEWBIK::get_root_bone() const {
	return root_bone;
}

void SkeletonModification3DEWBIK::set_root_bone(const StringName &p_root_bone) {
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
	int32_t old_count = pins.size();
	pin_count = p_value;
	pins.resize(p_value);
	for (int32_t pin_i = p_value; pin_i-- > old_count;) {
		pins.write[pin_i].instantiate();
	}
	notify_property_list_changed();
	is_dirty = true;
}

int32_t SkeletonModification3DEWBIK::get_pin_count() const {
	return pin_count;
}

void SkeletonModification3DEWBIK::add_pin(const StringName &p_name, const NodePath &p_target_node, const bool &p_use_node_rotation) {
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
	if (data.is_null()) {
		data.instantiate();
		pins.write[p_pin_index] = data;
	}
	data->set_name(p_bone);
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_pin_target_nodepath(int32_t p_pin_index, const NodePath &p_target_node) {
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

	if (!skeleton) {
		return;
	}
	update_skeleton();
	if (bone_list.size()) {
		Ref<IKTransform3D> bone_ik_transform = bone_list.write[0]->get_ik_transform();
		ERR_FAIL_NULL(bone_ik_transform);
		Ref<IKTransform3D> root_ik_transform = bone_ik_transform->get_parent();
		ERR_FAIL_NULL(root_ik_transform);
		root_ik_transform->set_global_transform(skeleton->get_global_transform());
	}
	update_shadow_bones_transform();
	double time_ms = OS::get_singleton()->get_ticks_msec() + get_time_budget_millisecond();
	ik_iterations = 0;
	do {
		segmented_skeleton->segment_solver(get_default_damp());
		ik_iterations++;
	} while (time_ms > OS::get_singleton()->get_ticks_msec() && ik_iterations < get_max_ik_iterations());
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
	notify_property_list_changed();
	update_skeleton();
	is_setup = true;
	is_dirty = false;
	execution_error_found = false;
}

void SkeletonModification3DEWBIK::update_skeleton() {
	if (!skeleton) {
		return;
	}
	if (!root_bone) {
		Vector<int32_t> roots = skeleton->get_parentless_bones();
		if (roots.size()) {
			set_root_bone_index(roots[0]);
		}
	} else if (root_bone_index == -1) {
		set_root_bone(root_bone);
	}
	ERR_FAIL_COND(!root_bone);
#ifdef TOOLS_ENABLED
	if (InspectorDock::get_inspector_singleton()->is_connected("edited_object_changed", callable_mp(this, &SkeletonModification3DEWBIK::set_dirty))) {
		InspectorDock::get_inspector_singleton()->disconnect("edited_object_changed", callable_mp(this, &SkeletonModification3DEWBIK::set_dirty));
	}
	InspectorDock::get_inspector_singleton()->connect("edited_object_changed", callable_mp(this, &SkeletonModification3DEWBIK::set_dirty));
#endif
	segmented_skeleton = Ref<IKBoneSegment>(memnew(IKBoneSegment(skeleton, skeleton->get_bone_name(root_bone_index), pins)));
	segmented_skeleton->get_root()->get_ik_transform()->set_parent(root_transform);
	segmented_skeleton->generate_default_segments_from_root(pins);
	bone_list.clear();
	segmented_skeleton->create_bone_list(bone_list, true, debug_skeleton);

	segmented_skeleton->update_pinned_list();
	for (int effector_i = 0; effector_i < get_pin_count(); effector_i++) {
		Ref<IKEffectorTemplate> data = pins.write[effector_i];
		if (data.is_null()) {
			continue;
		}
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
			float axial_from = get_kusudama_twist_from(constraint_i);
			float axial_to = get_kusudama_twist_to(constraint_i);
			constraint->set_axial_limits(axial_from, axial_to);
			if (get_kusudama_limit_cone_count(constraint_i)) {
				constraint->enable_orientational_limits();
			}
			for (int32_t cone_i = 0; cone_i < get_kusudama_limit_cone_count(constraint_i); cone_i++) {
				constraint->add_limit_cone(get_kusudama_limit_cone_center(constraint_i, cone_i), get_kusudama_limit_cone_radius(constraint_i, cone_i));
			}
			constraint->update_tangent_radii();
			constraint->update_rotational_freedom();
			constraint->optimize_limiting_axes();
			ik_bone_3d->addConstraint(constraint);
			break;
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
			bone->get_pin()->update_target_global_transform(skeleton);
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
					PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_limit_cone/" + itos(cone_i) + "/radius", PROPERTY_HINT_RANGE, "0,180,0.1,radians,exp"));
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

bool SkeletonModification3DEWBIK::_get(const StringName &p_name, Variant &r_ret) const {
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
				if (!get_kusudama_limit_cone_count(cone_index)) {
					r_ret = Vector3(0, 1, 0);
					return true;
				}
				r_ret = get_kusudama_limit_cone_center(index, cone_index);
				return true;
			} else if (cone_what == "radius") {
				if (!get_kusudama_limit_cone_count(cone_index)) {
					r_ret = 0.0f;
					return true;
				}
				r_ret = get_kusudama_limit_cone_radius(index, cone_index);
				return true;
			}
		}
	}

	return false;
}

bool SkeletonModification3DEWBIK::_set(const StringName &p_name, const Variant &p_value) {
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
				set_kusudama_limit_cone_center(index, cone_index, p_value);
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

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_kusudama_limit_cone_radius", "index", "cone_index", "radius"), &SkeletonModification3DEWBIK::set_kusudama_limit_cone_radius);
	ClassDB::bind_method(D_METHOD("get_kusudama_limit_cone_radius", "index", "cone_index"), &SkeletonModification3DEWBIK::get_kusudama_limit_cone_radius);
	ClassDB::bind_method(D_METHOD("set_kusudama_limit_cone_center", "index", "cone_index", "center"), &SkeletonModification3DEWBIK::set_kusudama_limit_cone_center);
	ClassDB::bind_method(D_METHOD("get_kusudama_limit_cone_center", "index", "cone_index"), &SkeletonModification3DEWBIK::set_kusudama_limit_cone_center);
	ClassDB::bind_method(D_METHOD("set_kusudama_limit_cone_count", "index", "count"), &SkeletonModification3DEWBIK::set_kusudama_limit_cone_count);
	ClassDB::bind_method(D_METHOD("get_kusudama_limit_cone_count", "index"), &SkeletonModification3DEWBIK::get_kusudama_limit_cone_count);
	ClassDB::bind_method(D_METHOD("set_kusudama_twist_from", "index", "name"), &SkeletonModification3DEWBIK::set_kusudama_twist_from);
	ClassDB::bind_method(D_METHOD("get_kusudama_twist_from", "index"), &SkeletonModification3DEWBIK::get_kusudama_twist_from);
	ClassDB::bind_method(D_METHOD("set_kusudama_twist_to", "index", "name"), &SkeletonModification3DEWBIK::set_kusudama_twist_to);
	ClassDB::bind_method(D_METHOD("get_kusudama_twist_to", "index"), &SkeletonModification3DEWBIK::get_kusudama_twist_to);
	ClassDB::bind_method(D_METHOD("set_pin_depth_falloff", "index", "falloff"), &SkeletonModification3DEWBIK::set_pin_depth_falloff);
	ClassDB::bind_method(D_METHOD("get_pin_depth_falloff", "index"), &SkeletonModification3DEWBIK::get_pin_depth_falloff);
	ClassDB::bind_method(D_METHOD("set_constraint_name", "index", "name"), &SkeletonModification3DEWBIK::set_constraint_name);
	ClassDB::bind_method(D_METHOD("get_constraint_name", "index"), &SkeletonModification3DEWBIK::get_constraint_name);
	ClassDB::bind_method(D_METHOD("get_segmented_skeleton"), &SkeletonModification3DEWBIK::get_segmented_skeleton);
	ClassDB::bind_method(D_METHOD("get_max_ik_iterations"), &SkeletonModification3DEWBIK::get_max_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_max_ik_iterations", "count"), &SkeletonModification3DEWBIK::set_max_ik_iterations);
	ClassDB::bind_method(D_METHOD("get_time_budget_millisecond"), &SkeletonModification3DEWBIK::get_time_budget_millisecond);
	ClassDB::bind_method(D_METHOD("set_time_budget_millisecond", "budget"), &SkeletonModification3DEWBIK::set_time_budget_millisecond);
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &SkeletonModification3DEWBIK::get_ik_iterations);
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
	ClassDB::bind_method(D_METHOD("get_kusudama_flip_handedness"), &SkeletonModification3DEWBIK::get_kusudama_flip_handedness);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "iterations", PROPERTY_HINT_NONE, ""), "", "get_ik_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_ik_iterations", PROPERTY_HINT_RANGE, "1,150,1,or_greater"), "set_max_ik_iterations", "get_max_ik_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "budget_millisecond", PROPERTY_HINT_RANGE, "0.01,2.0,0.01,or_greater"), "set_time_budget_millisecond", "get_time_budget_millisecond");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "root_bone", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_root_bone", "get_root_bone");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "default_damp", PROPERTY_HINT_RANGE, "0.04,179.99,0.01,radians,exp", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_default_damp", "get_default_damp");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "print_skeleton", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_debug_skeleton", "get_debug_skeleton");
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
	debug_skeleton = false;
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
	int32_t old_count = constraint_names.size();
	constraint_count = p_count;
	constraint_names.resize(p_count);
	kusudama_twist_from.resize(p_count);
	kusudama_twist_to.resize(p_count);
	kusudama_flip_handedness.resize(p_count);
	for (int32_t constraint_i = p_count; constraint_i-- > old_count;) {
		constraint_names.write[constraint_i] = String();
		kusudama_twist_from.write[constraint_i] = 0.0f;
		kusudama_twist_to.write[constraint_i] = Math::deg2rad(1.0f);
		kusudama_flip_handedness.write[constraint_i] = false;
	}
	notify_property_list_changed();
	is_dirty = true;
}

int32_t SkeletonModification3DEWBIK::get_constraint_count() const {
	return constraint_count;
}

inline StringName SkeletonModification3DEWBIK::get_constraint_name(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), StringName());
	return constraint_names[p_index];
}

void SkeletonModification3DEWBIK::set_kusudama_twist_from(int32_t p_index, float p_from) {
	ERR_FAIL_INDEX(p_index, constraint_count);
	kusudama_twist_from.write[p_index] = p_from;
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_kusudama_twist_to(int32_t p_index, float p_to) {
	ERR_FAIL_INDEX(p_index, constraint_count);
	kusudama_twist_to.write[p_index] = p_to;
	notify_property_list_changed();
	is_dirty = true;
}

int32_t SkeletonModification3DEWBIK::find_effector_id(StringName p_bone_name) {
	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		if (constraint_names[constraint_i] == p_bone_name) {
			return constraint_i;
		}
	}
	return -1;
}

void SkeletonModification3DEWBIK::set_kusudama_limit_cone(int32_t p_constraint, int32_t p_index,
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
	notify_property_list_changed();
	is_dirty = true;
}

Vector3 SkeletonModification3DEWBIK::get_kusudama_limit_cone_center(int32_t p_bone, int32_t p_index) const {
	ERR_FAIL_COND_V(!kusudama_limit_cones.has(p_bone), Vector3(0.0, 1.0, 0.0));
	const Color &cone = kusudama_limit_cones[p_bone][p_index];
	Vector3 ret;
	ret.x = cone.r;
	ret.y = cone.g;
	ret.z = cone.b;
	return ret;
}

float SkeletonModification3DEWBIK::get_kusudama_limit_cone_radius(int32_t p_bone, int32_t p_index) const {
	if (!kusudama_limit_cones.has(p_bone)) {
		return 0.0f;
	}
	return kusudama_limit_cones[p_bone][p_index].a;
}

int32_t SkeletonModification3DEWBIK::get_kusudama_limit_cone_count(int32_t p_effector) const {
	if (!kusudama_limit_cone_count.has(p_effector)) {
		return 0;
	}
	return kusudama_limit_cone_count[p_effector];
}

void SkeletonModification3DEWBIK::set_kusudama_limit_cone_count(int32_t p_effector, int32_t p_count) {
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
	notify_property_list_changed();
	is_dirty = true;
}

real_t SkeletonModification3DEWBIK::get_default_damp() const {
	return default_damp;
}

void SkeletonModification3DEWBIK::set_default_damp(float p_default_damp) {
	default_damp = p_default_damp;
	notify_property_list_changed();
	is_dirty = true;
}

StringName SkeletonModification3DEWBIK::get_pin_bone_name(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), "");
	Ref<IKEffectorTemplate> data = pins[p_effector_index];
	return data->get_name();
}

void SkeletonModification3DEWBIK::set_kusudama_limit_cone_radius(int32_t p_effector_index, int32_t p_index, float p_radius) {
	ERR_FAIL_COND(!kusudama_limit_cones.has(p_effector_index));
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_effector_index].size());
	PackedColorArray &cones = kusudama_limit_cones[p_effector_index];
	Color &cone = cones.write[p_index];
	cone.a = p_radius;

	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_kusudama_limit_cone_center(int32_t p_effector_index, int32_t p_index, Vector3 p_center) {
	ERR_FAIL_COND(!kusudama_limit_cones.has(p_effector_index));
	ERR_FAIL_INDEX(p_index, kusudama_limit_cones[p_effector_index].size());
	PackedColorArray &cones = kusudama_limit_cones[p_effector_index];
	Color &cone = cones.write[p_index];
	cone.r = p_center.x;
	cone.g = p_center.y;
	cone.b = p_center.z;

	notify_property_list_changed();
	is_dirty = true;
}

float SkeletonModification3DEWBIK::get_kusudama_twist_from(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, kusudama_twist_from.size(), 0.0f);
	return kusudama_twist_from[p_index];
}

float SkeletonModification3DEWBIK::get_kusudama_twist_to(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, kusudama_twist_to.size(), 360.0f);
	return kusudama_twist_to[p_index];
}

void SkeletonModification3DEWBIK::set_constraint_name(int32_t p_index, String p_name) {
	ERR_FAIL_INDEX(p_index, constraint_names.size());
	constraint_names.write[p_index] = p_name;
	notify_property_list_changed();
	is_dirty = true;
}

void SkeletonModification3DEWBIK::set_dirty() {
	is_dirty = true;
}

Ref<IKBoneSegment> SkeletonModification3DEWBIK::get_segmented_skeleton() {
	return segmented_skeleton;
}
float SkeletonModification3DEWBIK::get_max_ik_iterations() const {
	return max_ik_iterations;
}

void SkeletonModification3DEWBIK::set_max_ik_iterations(const float &p_max_ik_iterations) {
	max_ik_iterations = p_max_ik_iterations;
}

float SkeletonModification3DEWBIK::get_time_budget_millisecond() const {
	return time_budget_millisecond;
}

void SkeletonModification3DEWBIK::set_time_budget_millisecond(const float &p_time_budget) {
	time_budget_millisecond = p_time_budget;
}

bool SkeletonModification3DEWBIK::get_kusudama_flip_handedness(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, kusudama_flip_handedness.size(), false);
	return kusudama_flip_handedness[p_bone];
}

void SkeletonModification3DEWBIK::set_kusudama_flip_handedness(int32_t p_bone, bool p_flip) {
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