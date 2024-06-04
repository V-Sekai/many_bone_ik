extends SkeletonModifier3D

var is_constraint_mode = false
var skeleton_path = NodePath()
var segmented_skeletons: Array = []
var constraint_count = 0
var pin_count = 0
var bone_count = 0
var constraint_names: Array = []
var pins: Array = []
var bone_list: Array = []
var joint_twist: Array = []
var bone_damp: Array = []
var kusudama_open_cones: Array = []
var kusudama_open_cone_count: Array = []
var max_kusudama_open_cones = 10
var iterations_per_frame = 15
var default_damp = deg2rad(5.0)
var queue_debug_skeleton = false
var godot_skeleton_transform = null
var godot_skeleton_transform_inverse = Transform()
var ik_origin = null
var is_dirty = true
var skeleton_node_path = NodePath("..")
var ui_selected_bone = -1
var stabilize_passes = 0

func _on_timer_timeout():
    pass

func _update_ik_bones_transform():
    pass

func _update_skeleton_bones_transform():
    pass

func _get_bone_effectors() -> Array:
    return []

func set_constraint_name_at_index(p_index: int, p_name: String):
    pass

func set_total_effector_count(p_value: int):
    pass

func _set_constraint_count(p_count: int):
    pass

func _remove_pin(p_index: int):
    pass

func _set_bone_count(p_count: int):
    pass

func _set_pin_root_bone(p_pin_index: int, p_root_bone: String):
    pass

func _get_pin_root_bone(p_pin_index: int) -> String:
    return ""

func _bone_list_changed():
    pass

func _pose_updated():
    pass

func _update_ik_bone_pose(p_bone_idx: int):
    pass


func _set(p_name: StringName, p_value) -> bool:
    pass

func _get(p_name: StringName) -> Variant:
    return null

func _get_property_list() -> Array:
    return []

@staticmethod
func _bind_methods():
    pass

func _process_modification():
    pass

func _skeleton_changed(p_old, p_new):
    pass

func set_effector_target_fixed(p_effector_index: int, p_force_ignore: bool):
    pass

func get_effector_target_fixed(p_effector_index: int) -> bool:
    return false

func set_state(p_state):
    pass

func get_state():
    return null

func add_constraint():
    pass

func set_stabilization_passes(p_passes: int):
    pass

func get_stabilization_passes() -> int:
    return 0

func get_godot_skeleton_transform_inverse():
    return Transform3D()

func get_godot_skeleton_transform():
    return null

func set_ui_selected_bone(p_ui_selected_bone: int):
    pass

func get_ui_selected_bone() -> int:
    return 0

func set_constraint_mode(p_enabled: bool):
    pass

func get_constraint_mode() -> bool:
    return false

func get_pin_enabled(p_effector_index: int) -> bool:
    return false

func register_skeleton():
    pass

func reset_constraints():
    pass

func get_bone_list():
    return []

func get_segmented_skeletons():
    return []

func get_iterations_per_frame() -> float:
    return 0.0

func set_iterations_per_frame(p_iterations_per_frame: float):
    pass

func queue_print_skeleton():
    pass

func get_effector_count() -> int:
    return 0

func remove_constraint_at_index(p_index: int):
    pass

func set_effector_bone_name(p_pin_index: int, p_bone: String):
    pass

func get_effector_bone_name(p_effector_index: int) -> StringName:
    return ""

func set_effector_pin_node_path(p_effector_index: int, p_node_path):
    pass

func get_effector_pin_node_path(p_effector_index: int) -> NodePath:
    return NodePath()

func find_effector_id(p_bone_name: StringName) -> int:
    return 0

func set_effector_target_node_path(p_effector_index: int, p_target_node):
    pass

func set_pin_weight(p_pin_index: int, p_weight: float):
    pass

func get_pin_weight(p_pin_index: int) -> float:
    return 0.0

func set_pin_direction_priorities(p_pin_index: int, p_priority_direction):
    pass

func get_pin_direction_priorities(p_pin_index: int) -> Vector3:
    return Vector3()

func get_effector_target_node_path(p_pin_index: int) -> NodePath:
    return NodePath()

func set_pin_passthrough_factor(p_effector_index: int, p_passthrough_factor: float):
    pass

func get_pin_passthrough_factor(p_effector_index: int) -> float:
    return 0.0

func get_default_damp() -> float:
    return 0.0

func set_default_damp(p_default_damp: float):
    pass

func find_constraint(p_string: String) -> int:
    return 0

func find_pin(p_string: String) -> int:
    return 0

func get_constraint_count() -> int:
    return 0

func get_constraint_name(p_index: int) -> StringName:
    return ""

func set_twist_transform_of_constraint(p_index: int, p_transform):
    pass

func get_twist_transform_of_constraint(p_index: int) -> Transform3D:
    return Transform3D()

func set_orientation_transform_of_constraint(p_index: int, p_transform):
    pass

func get_orientation_transform_of_constraint(p_index: int) -> Transform3D:
    return Transform3D()

func set_direction_transform_of_bone(p_index: int, p_transform):
    pass

func get_direction_transform_of_bone(p_index: int) -> Transform3D:
    return Transform3D()

func get_joint_twist(p_index: int) -> Vector2:
    return Vector2()

func set_joint_twist(p_index: int, p_twist):
    pass

func set_kusudama_open_cone(p_bone: int, p_index: int, p_center, p_radius: float):
    pass

func get_kusudama_open_cone_center(p_constraint_index: int, p_index: int) -> Vector3:
    return Vector3()

func get_kusudama_open_cone_radius(p_constraint_index: int, p_index: int) -> float:
    return 0.0

func get_kusudama_open_cone_count(p_constraint_index: int) -> int:
    return 0

func get_bone_count() -> int:
    return 0

func set_kusudama_twist_from_to(p_index: int, from: float, to: float):
    pass

func set_kusudama_open_cone_count(p_constraint_index: int, p_count: int):
    pass

func set_kusudama_open_cone_center(p_constraint_index: int, p_index: int, p_center):
    pass

func set_kusudama_open_cone_radius(p_constraint_index: int, p_index: int, p_radius: float):
    pass

func _init():
    pass

func _exit_tree():
    pass

func set_dirty():
    pass

# void ManyBoneIK3D::set_total_effector_count(int32_t p_value) {
# 	int32_t old_count = pins.size();
# 	pin_count = p_value;
# 	pins.resize(p_value);
# 	for (int32_t pin_i = p_value; pin_i-- > old_count;) {
# 		pins.write[pin_i].instantiate();
# 	}
# 	set_dirty();
# }

# int32_t ManyBoneIK3D::get_effector_count() const {
# 	return pin_count;
# }

# void ManyBoneIK3D::set_effector_target_node_path(int32_t p_pin_index, const NodePath &p_target_node) {
# 	ERR_FAIL_INDEX(p_pin_index, pins.size());
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	if (effector_template.is_null()) {
# 		effector_template.instantiate();
# 		pins.write[p_pin_index] = effector_template;
# 	}
# 	effector_template->set_target_node(p_target_node);
# 	set_dirty();
# }

# NodePath ManyBoneIK3D::get_effector_target_node_path(int32_t p_pin_index) {
# 	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), NodePath());
# 	const Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	return effector_template->get_target_node();
# }

# Vector<Ref<IKEffectorTemplate3D>> ManyBoneIK3D::_get_bone_effectors() const {
# 	return pins;
# }

# void ManyBoneIK3D::_remove_pin(int32_t p_index) {
# 	ERR_FAIL_INDEX(p_index, pins.size());
# 	pins.remove_at(p_index);
# 	pin_count--;
# 	pins.resize(pin_count);
# 	set_dirty();
# }

# void ManyBoneIK3D::_update_ik_bones_transform() {
# 	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
# 		Ref<IKBone3D> bone = bone_list[bone_i];
# 		if (bone.is_null()) {
# 			continue;
# 		}
# 		bone->set_initial_pose(get_skeleton());
# 		if (bone->is_pinned()) {
# 			bone->get_pin()->update_target_global_transform(get_skeleton(), this);
# 		}
# 	}
# }

# void ManyBoneIK3D::_update_skeleton_bones_transform() {
# 	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
# 		Ref<IKBone3D> bone = bone_list[bone_i];
# 		if (bone.is_null()) {
# 			continue;
# 		}
# 		if (bone->get_bone_id() == -1) {
# 			continue;
# 		}
# 		bone->set_skeleton_bone_pose(get_skeleton());
# 	}
# 	update_gizmos();
# }

# void ManyBoneIK3D::_get_property_list(List<PropertyInfo> *p_list) const {
# 	const Vector<Ref<IKBone3D>> ik_bones = get_bone_list();
# 	RBSet<StringName> existing_pins;
# 	for (int32_t pin_i = 0; pin_i < get_effector_count(); pin_i++) {
# 		const String bone_name = get_effector_bone_name(pin_i);
# 		existing_pins.insert(bone_name);
# 	}
# 	const uint32_t pin_usage = PROPERTY_USAGE_DEFAULT;
# 	p_list->push_back(
# 			PropertyInfo(Variant::INT, "pin_count",
# 					PROPERTY_HINT_RANGE, "0,65536,or_greater", pin_usage | PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_READ_ONLY,
# 					"Pins,pins/"));
# 	for (int pin_i = 0; pin_i < get_effector_count(); pin_i++) {
# 		PropertyInfo effector_name;
# 		effector_name.type = Variant::STRING_NAME;
# 		effector_name.name = "pins/" + itos(pin_i) + "/bone_name";
# 		effector_name.usage = pin_usage;
# 		if (get_skeleton()) {
# 			String names;
# 			for (int bone_i = 0; bone_i < get_skeleton()->get_bone_count(); bone_i++) {
# 				String name = get_skeleton()->get_bone_name(bone_i);
# 				StringName string_name = StringName(name);
# 				if (existing_pins.has(string_name)) {
# 					continue;
# 				}
# 				name += ",";
# 				names += name;
# 				existing_pins.insert(name);
# 			}
# 			effector_name.hint = PROPERTY_HINT_ENUM_SUGGESTION;
# 			effector_name.hint_string = names;
# 		} else {
# 			effector_name.hint = PROPERTY_HINT_NONE;
# 			effector_name.hint_string = "";
# 		}
# 		p_list->push_back(effector_name);
# 		p_list->push_back(
# 				PropertyInfo(Variant::NODE_PATH, "pins/" + itos(pin_i) + "/target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D", pin_usage));
# 		p_list->push_back(
# 				PropertyInfo(Variant::BOOL, "pins/" + itos(pin_i) + "/target_static", PROPERTY_HINT_NONE, "", pin_usage));
# 		p_list->push_back(
# 				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/passthrough_factor", PROPERTY_HINT_RANGE, "0,1,0.1,or_greater", pin_usage));
# 		p_list->push_back(
# 				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/weight", PROPERTY_HINT_RANGE, "0,1,0.1,or_greater", pin_usage));
# 		p_list->push_back(
# 				PropertyInfo(Variant::VECTOR3, "pins/" + itos(pin_i) + "/direction_priorities", PROPERTY_HINT_RANGE, "0,1,0.1,or_greater", pin_usage));
# 	}
# 	uint32_t constraint_usage = PROPERTY_USAGE_DEFAULT;
# 	p_list->push_back(
# 			PropertyInfo(Variant::INT, "constraint_count",
# 					PROPERTY_HINT_RANGE, "0,256,or_greater", constraint_usage | PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_READ_ONLY,
# 					"Kusudama Constraints,constraints/"));
# 	RBSet<String> existing_constraints;
# 	for (int constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
# 		PropertyInfo bone_name;
# 		bone_name.type = Variant::STRING_NAME;
# 		bone_name.usage = constraint_usage;
# 		bone_name.name = "constraints/" + itos(constraint_i) + "/bone_name";
# 		if (get_skeleton()) {
# 			String names;
# 			for (int bone_i = 0; bone_i < get_skeleton()->get_bone_count(); bone_i++) {
# 				String name = get_skeleton()->get_bone_name(bone_i);
# 				if (existing_constraints.has(name)) {
# 					continue;
# 				}
# 				name += ",";
# 				names += name;
# 				existing_constraints.insert(name);
# 			}
# 			bone_name.hint = PROPERTY_HINT_ENUM_SUGGESTION;
# 			bone_name.hint_string = names;
# 		} else {
# 			bone_name.hint = PROPERTY_HINT_NONE;
# 			bone_name.hint_string = "";
# 		}
# 		p_list->push_back(bone_name);
# 		p_list->push_back(
# 				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/twist_start", PROPERTY_HINT_RANGE, "-359.9,359.9,0.1,radians,exp", constraint_usage));
# 		p_list->push_back(
# 				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/twist_end", PROPERTY_HINT_RANGE, "-359.9,359.9,0.1,radians,exp", constraint_usage));
# 		p_list->push_back(
# 				PropertyInfo(Variant::INT, "constraints/" + itos(constraint_i) + "/kusudama_open_cone_count", PROPERTY_HINT_RANGE, "0,10,1", constraint_usage | PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_READ_ONLY,
# 						"Limit Cones,constraints/" + itos(constraint_i) + "/kusudama_open_cone/"));
# 		for (int cone_i = 0; cone_i < get_kusudama_open_cone_count(constraint_i); cone_i++) {
# 			p_list->push_back(
# 					PropertyInfo(Variant::VECTOR3, "constraints/" + itos(constraint_i) + "/kusudama_open_cone/" + itos(cone_i) + "/center", PROPERTY_HINT_RANGE, "-1,1,0.1,exp", constraint_usage));

# 			p_list->push_back(
# 					PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_open_cone/" + itos(cone_i) + "/radius", PROPERTY_HINT_RANGE, "0,180,0.1,radians,exp", constraint_usage));
# 		}
# 		p_list->push_back(
# 				PropertyInfo(Variant::TRANSFORM3D, "constraints/" + itos(constraint_i) + "/kusudama_twist", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
# 		p_list->push_back(
# 				PropertyInfo(Variant::TRANSFORM3D, "constraints/" + itos(constraint_i) + "/kusudama_orientation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
# 		p_list->push_back(
# 				PropertyInfo(Variant::TRANSFORM3D, "constraints/" + itos(constraint_i) + "/bone_direction", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
# 	}
# }

# bool ManyBoneIK3D::_get(const StringName &p_name, Variant &r_ret) const {
# 	String name = p_name;
# 	if (name == "constraint_count") {
# 		r_ret = get_constraint_count();
# 		return true;
# 	} else if (name == "pin_count") {
# 		r_ret = get_effector_count();
# 		return true;
# 	} else if (name == "bone_count") {
# 		r_ret = get_bone_count();
# 		return true;
# 	} else if (name.begins_with("pins/")) {
# 		int index = name.get_slicec('/', 1).to_int();
# 		String what = name.get_slicec('/', 2);
# 		ERR_FAIL_INDEX_V(index, pins.size(), false);
# 		Ref<IKEffectorTemplate3D> effector_template = pins[index];
# 		ERR_FAIL_NULL_V(effector_template, false);
# 		if (what == "bone_name") {
# 			r_ret = effector_template->get_name();
# 			return true;
# 		} else if (what == "target_node") {
# 			r_ret = effector_template->get_target_node();
# 			return true;
# 		} else if (what == "target_static") {
# 			r_ret = effector_template->get_target_node().is_empty();
# 			return true;
# 		} else if (what == "passthrough_factor") {
# 			r_ret = get_pin_passthrough_factor(index);
# 			return true;
# 		} else if (what == "weight") {
# 			r_ret = get_pin_weight(index);
# 			return true;
# 		} else if (what == "direction_priorities") {
# 			r_ret = get_pin_direction_priorities(index);
# 			return true;
# 		}
# 	} else if (name.begins_with("constraints/")) {
# 		int index = name.get_slicec('/', 1).to_int();
# 		String what = name.get_slicec('/', 2);
# 		ERR_FAIL_INDEX_V(index, constraint_count, false);
# 		String begins = "constraints/" + itos(index) + "/kusudama_open_cone";
# 		if (what == "bone_name") {
# 			ERR_FAIL_INDEX_V(index, constraint_names.size(), false);
# 			r_ret = constraint_names[index];
# 			return true;
# 		} else if (what == "twist_start") {
# 			r_ret = get_joint_twist(index).x;
# 			return true;
# 		} else if (what == "twist_end") {
# 			r_ret = get_joint_twist(index).y;
# 			return true;
# 		} else if (what == "kusudama_open_cone_count") {
# 			r_ret = get_kusudama_open_cone_count(index);
# 			return true;
# 		} else if (name.begins_with(begins)) {
# 			int32_t cone_index = name.get_slicec('/', 3).to_int();
# 			String cone_what = name.get_slicec('/', 4);
# 			if (cone_what == "center") {
# 				Vector3 center = get_kusudama_open_cone_center(index, cone_index);
# 				r_ret = center;
# 				return true;
# 			} else if (cone_what == "radius") {
# 				r_ret = get_kusudama_open_cone_radius(index, cone_index);
# 				return true;
# 			}
# 		} else if (what == "bone_direction") {
# 			r_ret = get_direction_transform_of_bone(index);
# 			return true;
# 		} else if (what == "kusudama_orientation") {
# 			r_ret = get_orientation_transform_of_constraint(index);
# 			return true;
# 		} else if (what == "kusudama_twist") {
# 			r_ret = get_twist_transform_of_constraint(index);
# 			return true;
# 		}
# 	}
# 	return false;
# }

# bool ManyBoneIK3D::_set(const StringName &p_name, const Variant &p_value) {
# 	String name = p_name;
# 	if (name == "constraint_count") {
# 		_set_constraint_count(p_value);
# 		return true;
# 	} else if (name == "pin_count") {
# 		set_total_effector_count(p_value);
# 		return true;
# 	} else if (name.begins_with("pins/")) {
# 		int index = name.get_slicec('/', 1).to_int();
# 		String what = name.get_slicec('/', 2);
# 		if (index >= pins.size()) {
# 			set_total_effector_count(constraint_count);
# 		}
# 		if (what == "bone_name") {
# 			set_effector_bone_name(index, p_value);
# 			return true;
# 		} else if (what == "target_node") {
# 			set_effector_target_node_path(index, p_value);
# 			return true;
# 		} else if (what == "target_static") {
# 			if (p_value) {
# 				set_effector_target_node_path(index, NodePath());
# 			}
# 			return true;
# 		} else if (what == "passthrough_factor") {
# 			set_pin_passthrough_factor(index, p_value);
# 			return true;
# 		} else if (what == "weight") {
# 			set_pin_weight(index, p_value);
# 			return true;
# 		} else if (what == "direction_priorities") {
# 			set_pin_direction_priorities(index, p_value);
# 			return true;
# 		}
# 	} else if (name.begins_with("constraints/")) {
# 		int index = name.get_slicec('/', 1).to_int();
# 		String what = name.get_slicec('/', 2);
# 		String begins = "constraints/" + itos(index) + "/kusudama_open_cone/";
# 		if (index >= constraint_names.size()) {
# 			_set_constraint_count(constraint_count);
# 		}
# 		if (what == "bone_name") {
# 			set_constraint_name_at_index(index, p_value);
# 			return true;
# 		} else if (what == "twist_from") {
# 			Vector2 twist_from = get_joint_twist(index);
# 			set_joint_twist(index, Vector2(p_value, twist_from.y));
# 			return true;
# 		} else if (what == "twist_range") {
# 			Vector2 twist_range = get_joint_twist(index);
# 			set_joint_twist(index, Vector2(twist_range.x, p_value));
# 			return true;
# 		} else if (what == "kusudama_open_cone_count") {
# 			set_kusudama_open_cone_count(index, p_value);
# 			return true;
# 		} else if (name.begins_with(begins)) {
# 			int cone_index = name.get_slicec('/', 3).to_int();
# 			String cone_what = name.get_slicec('/', 4);
# 			if (cone_what == "center") {
# 				set_kusudama_open_cone_center(index, cone_index, p_value);
# 				return true;
# 			} else if (cone_what == "radius") {
# 				set_kusudama_open_cone_radius(index, cone_index, p_value);
# 				return true;
# 			}
# 		} else if (what == "bone_direction") {
# 			set_direction_transform_of_bone(index, p_value);
# 			return true;
# 		} else if (what == "kusudama_orientation") {
# 			set_orientation_transform_of_constraint(index, p_value);
# 			return true;
# 		} else if (what == "kusudama_twist") {
# 			set_twist_transform_of_constraint(index, p_value);
# 			return true;
# 		}
# 	}

# 	return false;
# }

# void ManyBoneIK3D::queue_print_skeleton() {
# 	queue_debug_skeleton = true;
# }

# float ManyBoneIK3D::get_pin_passthrough_factor(int32_t p_effector_index) const {
# 	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), 0.0f);
# 	const Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	return effector_template->get_passthrough_factor();
# }

# void ManyBoneIK3D::set_pin_passthrough_factor(int32_t p_effector_index, const float p_passthrough_factor) {
# 	ERR_FAIL_INDEX(p_effector_index, pins.size());
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	ERR_FAIL_NULL(effector_template);
# 	effector_template->set_passthrough_factor(p_passthrough_factor);
# 	set_dirty();
# }

# void ManyBoneIK3D::_set_constraint_count(int32_t p_count) {
# 	int32_t old_count = constraint_names.size();
# 	constraint_count = p_count;
# 	constraint_names.resize(p_count);
# 	joint_twist.resize(p_count);
# 	kusudama_open_cone_count.resize(p_count);
# 	kusudama_open_cones.resize(p_count);
# 	for (int32_t constraint_i = p_count; constraint_i-- > old_count;) {
# 		constraint_names.write[constraint_i] = String();
# 		kusudama_open_cone_count.write[constraint_i] = 0;
# 		kusudama_open_cones.write[constraint_i].resize(1);
# 		kusudama_open_cones.write[constraint_i].write[0] = Vector4(0, 1, 0, 0.01745f);
# 		joint_twist.write[constraint_i] = Vector2(0, 0.01745f);
# 	}
# 	set_dirty();
# 	notify_property_list_changed();
# }

# int32_t ManyBoneIK3D::get_constraint_count() const {
# 	return constraint_count;
# }

# inline StringName ManyBoneIK3D::get_constraint_name(int32_t p_index) const {
# 	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), StringName());
# 	return constraint_names[p_index];
# }

# Vector2 ManyBoneIK3D::get_joint_twist(int32_t p_index) const {
# 	ERR_FAIL_INDEX_V(p_index, joint_twist.size(), Vector2());
# 	return joint_twist[p_index];
# }

# void ManyBoneIK3D::set_joint_twist(int32_t p_index, Vector2 p_to) {
# 	ERR_FAIL_INDEX(p_index, constraint_count);
# 	joint_twist.write[p_index] = p_to;
# 	set_dirty();
# }

# int32_t ManyBoneIK3D::find_effector_id(StringName p_bone_name) {
# 	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
# 		if (constraint_names[constraint_i] == p_bone_name) {
# 			return constraint_i;
# 		}
# 	}
# 	return -1;
# }

# void ManyBoneIK3D::set_kusudama_open_cone(int32_t p_constraint_index, int32_t p_index,
# 		Vector3 p_center, float p_radius) {
# 	ERR_FAIL_INDEX(p_constraint_index, kusudama_open_cones.size());
# 	Vector<Vector4> cones = kusudama_open_cones.write[p_constraint_index];
# 	if (Math::is_zero_approx(p_center.length_squared())) {
# 		p_center = Vector3(0.0f, 1.0f, 0.0f);
# 	}
# 	Vector3 center = p_center.normalized();
# 	Vector4 cone;
# 	cone.x = center.x;
# 	cone.y = center.y;
# 	cone.z = center.z;
# 	cone.w = p_radius;
# 	cones.write[p_index] = cone;
# 	kusudama_open_cones.write[p_constraint_index] = cones;
# 	set_dirty();
# }

# float ManyBoneIK3D::get_kusudama_open_cone_radius(int32_t p_constraint_index, int32_t p_index) const {
# 	ERR_FAIL_INDEX_V(p_constraint_index, kusudama_open_cones.size(), Math_TAU);
# 	ERR_FAIL_INDEX_V(p_index, kusudama_open_cones[p_constraint_index].size(), Math_TAU);
# 	return kusudama_open_cones[p_constraint_index][p_index].w;
# }

# int32_t ManyBoneIK3D::get_kusudama_open_cone_count(int32_t p_constraint_index) const {
# 	ERR_FAIL_INDEX_V(p_constraint_index, kusudama_open_cone_count.size(), 0);
# 	return kusudama_open_cone_count[p_constraint_index];
# }

# void ManyBoneIK3D::set_kusudama_open_cone_count(int32_t p_constraint_index, int32_t p_count) {
# 	ERR_FAIL_INDEX(p_constraint_index, kusudama_open_cone_count.size());
# 	ERR_FAIL_INDEX(p_constraint_index, kusudama_open_cones.size());
# 	int32_t old_cone_count = kusudama_open_cones[p_constraint_index].size();
# 	kusudama_open_cone_count.write[p_constraint_index] = p_count;
# 	Vector<Vector4> &cones = kusudama_open_cones.write[p_constraint_index];
# 	cones.resize(p_count);
# 	String bone_name = get_constraint_name(p_constraint_index);
# 	Transform3D bone_transform = get_direction_transform_of_bone(p_constraint_index);
# 	Vector3 forward_axis = -bone_transform.basis.get_column(Vector3::AXIS_Y).normalized();
# 	for (int32_t cone_i = p_count; cone_i-- > old_cone_count;) {
# 		Vector4 &cone = cones.write[cone_i];
# 		cone.x = forward_axis.x;
# 		cone.y = forward_axis.y;
# 		cone.z = forward_axis.z;
# 		cone.w = Math::deg_to_rad(0.0f);
# 	}
# 	set_dirty();
# 	notify_property_list_changed();
# }

# real_t ManyBoneIK3D::get_default_damp() const {
# 	return default_damp;
# }

# void ManyBoneIK3D::set_default_damp(float p_default_damp) {
# 	default_damp = p_default_damp;
# 	set_dirty();
# }

# StringName ManyBoneIK3D::get_effector_bone_name(int32_t p_effector_index) const {
# 	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), "");
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	return effector_template->get_name();
# }

# void ManyBoneIK3D::set_kusudama_open_cone_radius(int32_t p_effector_index, int32_t p_index, float p_radius) {
# 	ERR_FAIL_INDEX(p_effector_index, kusudama_open_cone_count.size());
# 	ERR_FAIL_INDEX(p_effector_index, kusudama_open_cones.size());
# 	ERR_FAIL_INDEX(p_index, kusudama_open_cone_count[p_effector_index]);
# 	ERR_FAIL_INDEX(p_index, kusudama_open_cones[p_effector_index].size());
# 	Vector4 &cone = kusudama_open_cones.write[p_effector_index].write[p_index];
# 	cone.w = p_radius;
# 	set_dirty();
# }

# void ManyBoneIK3D::set_kusudama_open_cone_center(int32_t p_effector_index, int32_t p_index, Vector3 p_center) {
# 	ERR_FAIL_INDEX(p_effector_index, kusudama_open_cones.size());
# 	ERR_FAIL_INDEX(p_index, kusudama_open_cones[p_effector_index].size());
# 	Vector4 &cone = kusudama_open_cones.write[p_effector_index].write[p_index];
# 	Basis basis;
# 	basis.set_column(0, Vector3(1, 0, 0));
# 	basis.set_column(1, Vector3(0, 0, -1));
# 	basis.set_column(2, Vector3(0, 1, 0));
# 	if (Math::is_zero_approx(p_center.length_squared())) {
# 		cone.x = 0;
# 		cone.y = 0;
# 		cone.z = 1;
# 	} else {
# 		p_center = basis.xform(p_center);
# 		cone.x = p_center.x;
# 		cone.y = p_center.y;
# 		cone.z = p_center.z;
# 	}
# 	set_dirty();
# }

# Vector3 ManyBoneIK3D::get_kusudama_open_cone_center(int32_t p_constraint_index, int32_t p_index) const {
# 	if (unlikely((p_constraint_index) < 0 || (p_constraint_index) >= (kusudama_open_cones.size()))) {
# 		ERR_PRINT_ONCE("Can't get limit cone center.");
# 		return Vector3(0.0, 0.0, 1.0);
# 	}
# 	if (unlikely((p_index) < 0 || (p_index) >= (kusudama_open_cones[p_constraint_index].size()))) {
# 		ERR_PRINT_ONCE("Can't get limit cone center.");
# 		return Vector3(0.0, 0.0, 1.0);
# 	}
# 	const Vector4 &cone = kusudama_open_cones[p_constraint_index][p_index];
# 	Vector3 ret;
# 	ret.x = cone.x;
# 	ret.y = cone.y;
# 	ret.z = cone.z;
# 	Basis basis;
# 	basis.set_column(0, Vector3(1, 0, 0));
# 	basis.set_column(1, Vector3(0, 0, -1));
# 	basis.set_column(2, Vector3(0, 1, 0));
# 	return basis.xform_inv(ret);
# }

# void ManyBoneIK3D::set_constraint_name_at_index(int32_t p_index, String p_name) {
# 	ERR_FAIL_INDEX(p_index, constraint_names.size());
# 	constraint_names.write[p_index] = p_name;
# 	set_dirty();
# }

# Vector<Ref<IKBoneSegment3D>> ManyBoneIK3D::get_segmented_skeletons() {
# 	return segmented_skeletons;
# }

# float ManyBoneIK3D::get_iterations_per_frame() const {
# 	return iterations_per_frame;
# }

# void ManyBoneIK3D::set_iterations_per_frame(const float &p_iterations_per_frame) {
# 	iterations_per_frame = p_iterations_per_frame;
# }

# void ManyBoneIK3D::set_effector_pin_node_path(int32_t p_effector_index, NodePath p_node_path) {
# 	ERR_FAIL_INDEX(p_effector_index, pins.size());
# 	Node *node = get_node_or_null(p_node_path);
# 	if (!node) {
# 		return;
# 	}
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	effector_template->set_target_node(p_node_path);
# }

# NodePath ManyBoneIK3D::get_effector_pin_node_path(int32_t p_effector_index) const {
# 	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), NodePath());
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	return effector_template->get_target_node();
# }

# void ManyBoneIK3D::_process_modification() {
# 	if (!get_skeleton()) {
# 		return;
# 	}
# 	if (get_effector_count() == 0) {
# 		return;
# 	}
# 	if (!segmented_skeletons.size()) {
# 		set_dirty();
# 	}
# 	if (is_dirty) {
# 		is_dirty = false;
# 		_bone_list_changed();
# 	}
# 	if (bone_list.size()) {
# 		Ref<IKNode3D> root_ik_bone = bone_list.write[0]->get_ik_transform();
# 		if (root_ik_bone.is_null()) {
# 			return;
# 		}
# 		Skeleton3D *skeleton = get_skeleton();
# 		godot_skeleton_transform.instantiate();
# 		godot_skeleton_transform->set_transform(skeleton->get_transform());
# 		godot_skeleton_transform_inverse = skeleton->get_transform().affine_inverse();
# 	}
# 	bool has_pins = false;
# 	for (Ref<IKEffectorTemplate3D> pin : pins) {
# 		if (pin.is_valid() && !pin->get_name().is_empty()) {
# 			has_pins = true;
# 			break;
# 		}
# 	}
# 	if (!has_pins) {
# 		return;
# 	}
# 	if (!is_enabled()) {
# 		return;
# 	}
# 	if (!is_visible()) {
# 		return;
# 	}
# 	for (int32_t i = 0; i < get_iterations_per_frame(); i++) {
# 		for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
# 			if (segmented_skeleton.is_null()) {
# 				continue;
# 			}
# 			segmented_skeleton->segment_solver(bone_damp, get_default_damp(), get_constraint_mode(), i, get_iterations_per_frame());
# 		}
# 	}
# 	_update_skeleton_bones_transform();
# }

# real_t ManyBoneIK3D::get_pin_weight(int32_t p_pin_index) const {
# 	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), 0.0);
# 	const Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	return effector_template->get_weight();
# }

# void ManyBoneIK3D::set_pin_weight(int32_t p_pin_index, const real_t &p_weight) {
# 	ERR_FAIL_INDEX(p_pin_index, pins.size());
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	if (effector_template.is_null()) {
# 		effector_template.instantiate();
# 		pins.write[p_pin_index] = effector_template;
# 	}
# 	effector_template->set_weight(p_weight);
# 	set_dirty();
# }

# Vector3 ManyBoneIK3D::get_pin_direction_priorities(int32_t p_pin_index) const {
# 	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), Vector3(0, 0, 0));
# 	const Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	return effector_template->get_direction_priorities();
# }

# void ManyBoneIK3D::set_pin_direction_priorities(int32_t p_pin_index, const Vector3 &p_priority_direction) {
# 	ERR_FAIL_INDEX(p_pin_index, pins.size());
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	if (effector_template.is_null()) {
# 		effector_template.instantiate();
# 		pins.write[p_pin_index] = effector_template;
# 	}
# 	effector_template->set_direction_priorities(p_priority_direction);
# 	set_dirty();
# }

# void ManyBoneIK3D::set_dirty() {
# 	is_dirty = true;
# }

# int32_t ManyBoneIK3D::find_constraint(String p_string) const {
# 	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
# 		if (get_constraint_name(constraint_i) == p_string) {
# 			return constraint_i;
# 		}
# 	}
# 	return -1;
# }

# void ManyBoneIK3D::remove_constraint_at_index(int32_t p_index) {
# 	ERR_FAIL_INDEX(p_index, constraint_count);

# 	constraint_names.remove_at(p_index);
# 	kusudama_open_cone_count.remove_at(p_index);
# 	kusudama_open_cones.remove_at(p_index);
# 	joint_twist.remove_at(p_index);

# 	constraint_count--;

# 	set_dirty();
# }

# void ManyBoneIK3D::_set_bone_count(int32_t p_count) {
# 	bone_damp.resize(p_count);
# 	for (int32_t bone_i = p_count; bone_i-- > bone_count;) {
# 		bone_damp.write[bone_i] = get_default_damp();
# 	}
# 	bone_count = p_count;
# 	set_dirty();
# 	notify_property_list_changed();
# }

# int32_t ManyBoneIK3D::get_bone_count() const {
# 	return bone_count;
# }

# Vector<Ref<IKBone3D>> ManyBoneIK3D::get_bone_list() const {
# 	return bone_list;
# }

# void ManyBoneIK3D::set_direction_transform_of_bone(int32_t p_index, Transform3D p_transform) {
# 	ERR_FAIL_INDEX(p_index, constraint_names.size());
# 	if (!get_skeleton()) {
# 		return;
# 	}
# 	String bone_name = constraint_names[p_index];
# 	int32_t bone_index = get_skeleton()->find_bone(bone_name);
# 	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
# 		if (segmented_skeleton.is_null()) {
# 			continue;
# 		}
# 		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(bone_index);
# 		if (ik_bone.is_null() || ik_bone->get_constraint().is_null()) {
# 			continue;
# 		}
# 		if (ik_bone->get_bone_direction_transform().is_null()) {
# 			continue;
# 		}
# 		ik_bone->get_bone_direction_transform()->set_transform(p_transform);
# 		break;
# 	}
# }

# Transform3D ManyBoneIK3D::get_direction_transform_of_bone(int32_t p_index) const {
# 	if (p_index < 0 || p_index >= constraint_names.size() || get_skeleton() == nullptr) {
# 		return Transform3D();
# 	}

# 	String bone_name = constraint_names[p_index];
# 	int32_t bone_index = get_skeleton()->find_bone(bone_name);
# 	for (const Ref<IKBoneSegment3D> &segmented_skeleton : segmented_skeletons) {
# 		if (segmented_skeleton.is_null()) {
# 			continue;
# 		}
# 		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(bone_index);
# 		if (ik_bone.is_null() || ik_bone->get_constraint().is_null()) {
# 			continue;
# 		}
# 		return ik_bone->get_bone_direction_transform()->get_transform();
# 	}
# 	return Transform3D();
# }

# Transform3D ManyBoneIK3D::get_orientation_transform_of_constraint(int32_t p_index) const {
# 	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), Transform3D());
# 	String bone_name = constraint_names[p_index];
# 	if (!segmented_skeletons.size()) {
# 		return Transform3D();
# 	}
# 	if (!get_skeleton()) {
# 		return Transform3D();
# 	}
# 	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
# 		if (segmented_skeleton.is_null()) {
# 			continue;
# 		}
# 		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
# 		if (ik_bone.is_null()) {
# 			continue;
# 		}
# 		if (ik_bone->get_constraint().is_null()) {
# 			continue;
# 		}
# 		return ik_bone->get_constraint_orientation_transform()->get_transform();
# 	}
# 	return Transform3D();
# }

# void ManyBoneIK3D::set_orientation_transform_of_constraint(int32_t p_index, Transform3D p_transform) {
# 	ERR_FAIL_INDEX(p_index, constraint_names.size());
# 	String bone_name = constraint_names[p_index];
# 	if (!get_skeleton()) {
# 		return;
# 	}
# 	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
# 		if (segmented_skeleton.is_null()) {
# 			continue;
# 		}
# 		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
# 		if (ik_bone.is_null()) {
# 			continue;
# 		}
# 		if (ik_bone->get_constraint().is_null()) {
# 			continue;
# 		}
# 		ik_bone->get_constraint_orientation_transform()->set_transform(p_transform);
# 		break;
# 	}
# }

# Transform3D ManyBoneIK3D::get_twist_transform_of_constraint(int32_t p_index) const {
# 	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), Transform3D());
# 	String bone_name = constraint_names[p_index];
# 	if (!segmented_skeletons.size()) {
# 		return Transform3D();
# 	}
# 	if (!get_skeleton()) {
# 		return Transform3D();
# 	}
# 	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
# 		if (segmented_skeleton.is_null()) {
# 			continue;
# 		}
# 		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
# 		if (ik_bone.is_null()) {
# 			continue;
# 		}
# 		if (ik_bone->get_constraint().is_null()) {
# 			continue;
# 		}
# 		return ik_bone->get_constraint_twist_transform()->get_transform();
# 	}
# 	return Transform3D();
# }

# void ManyBoneIK3D::set_twist_transform_of_constraint(int32_t p_index, Transform3D p_transform) {
# 	ERR_FAIL_INDEX(p_index, constraint_names.size());
# 	String bone_name = constraint_names[p_index];
# 	if (!get_skeleton()) {
# 		return;
# 	}
# 	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
# 		if (segmented_skeleton.is_null()) {
# 			continue;
# 		}
# 		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
# 		if (ik_bone.is_null()) {
# 			continue;
# 		}
# 		if (ik_bone->get_constraint().is_null()) {
# 			continue;
# 		}
# 		ik_bone->get_constraint_twist_transform()->set_transform(p_transform);
# 		break;
# 	}
# }

# bool ManyBoneIK3D::get_pin_enabled(int32_t p_effector_index) const {
# 	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), false);
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	if (effector_template->get_target_node().is_empty()) {
# 		return true;
# 	}
# 	return !effector_template->get_target_node().is_empty();
# }

# void ManyBoneIK3D::register_skeleton() {
# 	if (!get_effector_count() && !get_constraint_count()) {
# 		reset_constraints();
# 	}
# 	set_dirty();
# }

# void ManyBoneIK3D::reset_constraints() {
# 	Skeleton3D *skeleton = get_skeleton();
# 	if (skeleton) {
# 		int32_t saved_pin_count = get_effector_count();
# 		set_total_effector_count(0);
# 		set_total_effector_count(saved_pin_count);
# 		int32_t saved_constraint_count = constraint_names.size();
# 		_set_constraint_count(0);
# 		_set_constraint_count(saved_constraint_count);
# 		_set_bone_count(0);
# 		_set_bone_count(saved_constraint_count);
# 	}
# 	set_dirty();
# }

# void ManyBoneIK3D::set_stabilization_passes(int32_t p_passes) {
# 	stabilize_passes = p_passes;
# 	set_dirty();
# }


# void ManyBoneIK3D::add_constraint() {
# 	int32_t old_count = constraint_count;
# 	_set_constraint_count(constraint_count + 1);
# 	constraint_names.write[old_count] = String();
# 	kusudama_open_cone_count.write[old_count] = 0;
# 	kusudama_open_cones.write[old_count].resize(1);
# 	kusudama_open_cones.write[old_count].write[0] = Vector4(0, 1, 0, Math_PI);
# 	joint_twist.write[old_count] = Vector2(0, Math_PI);
# 	set_dirty();
# }

# int32_t ManyBoneIK3D::find_pin(String p_string) const {
# 	for (int32_t pin_i = 0; pin_i < pin_count; pin_i++) {
# 		if (get_effector_bone_name(pin_i) == p_string) {
# 			return pin_i;
# 		}
# 	}
# 	return -1;
# }

# bool ManyBoneIK3D::get_effector_target_fixed(int32_t p_effector_index) {
# 	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), false);
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	return get_effector_pin_node_path(p_effector_index).is_empty();
# }

# void ManyBoneIK3D::set_effector_target_fixed(int32_t p_effector_index, bool p_force_ignore) {
# 	ERR_FAIL_INDEX(p_effector_index, pins.size());
# 	if (!p_force_ignore) {
# 		return;
# 	}
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
# 	effector_template->set_target_node(NodePath());
# 	set_dirty();
# }

# void ManyBoneIK3D::_bone_list_changed() {
# 	Skeleton3D *skeleton = get_skeleton();
# 	Vector<int32_t> roots = skeleton->get_parentless_bones();
# 	if (roots.is_empty()) {
# 		return;
# 	}
# 	bone_list.clear();
# 	segmented_skeletons.clear();
# 	for (BoneId root_bone_index : roots) {
# 		String parentless_bone = skeleton->get_bone_name(root_bone_index);
# 		Ref<IKBoneSegment3D> segmented_skeleton = Ref<IKBoneSegment3D>(memnew(IKBoneSegment3D(skeleton, parentless_bone, pins, this, nullptr, root_bone_index, -1, stabilize_passes)));
# 		ik_origin.instantiate();
# 		segmented_skeleton->get_root()->get_ik_transform()->set_parent(ik_origin);
# 		segmented_skeleton->generate_default_segments(pins, root_bone_index, -1, this);
# 		Vector<Ref<IKBone3D>> new_bone_list;
# 		segmented_skeleton->create_bone_list(new_bone_list, true, queue_debug_skeleton);
# 		bone_list.append_array(new_bone_list);
# 		Vector<Vector<double>> weight_array;
# 		segmented_skeleton->update_pinned_list(weight_array);
# 		segmented_skeleton->recursive_create_headings_arrays_for(segmented_skeleton);
# 		segmented_skeletons.push_back(segmented_skeleton);
# 	}
# 	_update_ik_bones_transform();
# 	for (Ref<IKBone3D> &ik_bone_3d : bone_list) {
# 		ik_bone_3d->update_default_bone_direction_transform(skeleton);
# 	}
# 	for (int constraint_i = 0; constraint_i < constraint_count; ++constraint_i) {
# 		String bone = constraint_names[constraint_i];
# 		BoneId bone_id = skeleton->find_bone(bone);
# 		for (Ref<IKBone3D> &ik_bone_3d : bone_list) {
# 			if (ik_bone_3d->get_bone_id() != bone_id) {
# 				continue;
# 			}
# 			Ref<IKKusudama3D> constraint;
# 			constraint.instantiate();
# 			constraint->enable_orientational_limits();

# 			int32_t cone_count = kusudama_open_cone_count[constraint_i];
# 			const Vector<Vector4> &cones = kusudama_open_cones[constraint_i];
# 			for (int32_t cone_i = 0; cone_i < cone_count; ++cone_i) {
# 				const Vector4 &cone = cones[cone_i];
# 				Ref<IKOpenCone3D> new_cone;
# 				new_cone.instantiate();
# 				new_cone->set_attached_to(constraint);
# 				new_cone->set_tangent_circle_center_next_1(Vector3(0.0f, -1.0f, 0.0f));
# 				new_cone->set_tangent_circle_center_next_2(Vector3(0.0f, 1.0f, 0.0f));
# 				new_cone->set_radius(MAX(1.0e-38, cone.w));
# 				new_cone->set_control_point(Vector3(cone.x, cone.y, cone.z).normalized());
# 				constraint->add_open_cone(new_cone);
# 			}

# 			const Vector2 axial_limit = get_joint_twist(constraint_i);
# 			constraint->enable_axial_limits();
# 			constraint->set_axial_limits(axial_limit.x, axial_limit.y);
# 			ik_bone_3d->add_constraint(constraint);
# 			constraint->_update_constraint(ik_bone_3d->get_constraint_twist_transform());
# 			break;
# 		}
# 	}
# 	if (queue_debug_skeleton) {
# 		queue_debug_skeleton = false;
# 	}
# }

# void ManyBoneIK3D::_skeleton_changed(Skeleton3D *p_old, Skeleton3D *p_new) {
# 	if (p_old) {
# 		if (p_old->is_connected(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed))) {
# 			p_old->disconnect(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed));
# 		}
# 	}
# 	if (p_new) {
# 		if (!p_new->is_connected(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed))) {
# 			p_new->connect(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed));
# 		}
# 	}
# 	if (is_connected(SNAME("modification_processed"), callable_mp(this, &ManyBoneIK3D::_update_ik_bones_transform))) {
# 		disconnect(SNAME("modification_processed"), callable_mp(this, &ManyBoneIK3D::_update_ik_bones_transform));
# 	}
# 	connect(SNAME("modification_processed"), callable_mp(this, &ManyBoneIK3D::_update_ik_bones_transform));
# 	_bone_list_changed();
# }

# void ManyBoneIK3D::set_effector_bone_name(int32_t p_pin_index, const String &p_bone) {
# 	ERR_FAIL_INDEX(p_pin_index, pins.size());
# 	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
# 	if (effector_template.is_null()) {
# 		effector_template.instantiate();
# 		pins.write[p_pin_index] = effector_template;
# 	}
# 	effector_template->set_name(p_bone);
# 	set_dirty();
# }

