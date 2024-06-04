class ManyBoneIK3DState;
class ManyBoneIK3D : public SkeletonModifier3D {
	GDCLASS(ManyBoneIK3D, SkeletonModifier3D);

	bool is_constraint_mode = false;
	NodePath skeleton_path;
	Vector<Ref<IKBoneSegment3D>> segmented_skeletons;
	int32_t constraint_count = 0, pin_count = 0, bone_count = 0;
	Vector<StringName> constraint_names;
	Vector<Ref<IKEffectorTemplate3D>> pins;
	Vector<Ref<IKBone3D>> bone_list;
	Vector<Vector2> joint_twist;
	Vector<float> bone_damp;
	Vector<Vector<Vector4>> kusudama_open_cones;
	Vector<int> kusudama_open_cone_count;
	float MAX_KUSUDAMA_OPEN_CONES = 10;
	int32_t iterations_per_frame = 15;
	float default_damp = Math::deg_to_rad(5.0f);
	bool queue_debug_skeleton = false;
	Ref<IKNode3D> godot_skeleton_transform;
	Transform3D godot_skeleton_transform_inverse;
	Ref<IKNode3D> ik_origin;
	bool is_dirty = true;
	NodePath skeleton_node_path = NodePath("..");
	int32_t ui_selected_bone = -1, stabilize_passes = 0;

	void _on_timer_timeout();
	void _update_ik_bones_transform();
	void _update_skeleton_bones_transform();
	Vector<Ref<IKEffectorTemplate3D>> _get_bone_effectors() const;
	void set_constraint_name_at_index(int32_t p_index, String p_name);
	void set_total_effector_count(int32_t p_value);
	void _set_constraint_count(int32_t p_count);
	void _remove_pin(int32_t p_index);
	void _set_bone_count(int32_t p_count);
	void _set_pin_root_bone(int32_t p_pin_index, const String &p_root_bone);
	String _get_pin_root_bone(int32_t p_pin_index) const;
	void _bone_list_changed();
	void _pose_updated();
	void _update_ik_bone_pose(int32_t p_bone_idx);

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	static void _bind_methods();
	virtual void _process_modification() override;
	void _skeleton_changed(Skeleton3D *p_old, Skeleton3D *p_new) override;

public:
	void set_effector_target_fixed(int32_t p_effector_index, bool p_force_ignore);
	bool get_effector_target_fixed(int32_t p_effector_index);
	void set_state(Ref<ManyBoneIK3DState> p_state);
	Ref<ManyBoneIK3DState> get_state() const;
	void add_constraint();
	void set_stabilization_passes(int32_t p_passes);
	int32_t get_stabilization_passes();
	Transform3D get_godot_skeleton_transform_inverse();
	Ref<IKNode3D> get_godot_skeleton_transform();
	void set_ui_selected_bone(int32_t p_ui_selected_bone);
	int32_t get_ui_selected_bone() const;
	void set_constraint_mode(bool p_enabled);
	bool get_constraint_mode() const;
	bool get_pin_enabled(int32_t p_effector_index) const;
	void register_skeleton();
	void reset_constraints();
	Vector<Ref<IKBone3D>> get_bone_list() const;
	Vector<Ref<IKBoneSegment3D>> get_segmented_skeletons();
	float get_iterations_per_frame() const;
	void set_iterations_per_frame(const float &p_iterations_per_frame);
	void queue_print_skeleton();
	int32_t get_effector_count() const;
	void remove_constraint_at_index(int32_t p_index);
	void set_effector_bone_name(int32_t p_pin_index, const String &p_bone);
	StringName get_effector_bone_name(int32_t p_effector_index) const;
	void set_effector_pin_node_path(int32_t p_effector_index, NodePath p_node_path);
	NodePath get_effector_pin_node_path(int32_t p_effector_index) const;
	int32_t find_effector_id(StringName p_bone_name);
	void set_effector_target_node_path(int32_t p_effector_index, const NodePath &p_target_node);
	void set_pin_weight(int32_t p_pin_index, const real_t &p_weight);
	real_t get_pin_weight(int32_t p_pin_index) const;
	void set_pin_direction_priorities(int32_t p_pin_index, const Vector3 &p_priority_direction);
	Vector3 get_pin_direction_priorities(int32_t p_pin_index) const;
	NodePath get_effector_target_node_path(int32_t p_pin_index);
	void set_pin_passthrough_factor(int32_t p_effector_index, const float p_passthrough_factor);
	float get_pin_passthrough_factor(int32_t p_effector_index) const;
	real_t get_default_damp() const;
	void set_default_damp(float p_default_damp);
	int32_t find_constraint(String p_string) const;
	int32_t find_pin(String p_string) const;
	int32_t get_constraint_count() const;
	StringName get_constraint_name(int32_t p_index) const;
	void set_twist_transform_of_constraint(int32_t p_index, Transform3D p_transform);
	Transform3D get_twist_transform_of_constraint(int32_t p_index) const;
	void set_orientation_transform_of_constraint(int32_t p_index, Transform3D p_transform);
	Transform3D get_orientation_transform_of_constraint(int32_t p_index) const;
	void set_direction_transform_of_bone(int32_t p_index, Transform3D p_transform);
	Transform3D get_direction_transform_of_bone(int32_t p_index) const;
	Vector2 get_joint_twist(int32_t p_index) const;
	void set_joint_twist(int32_t p_index, Vector2 p_twist);
	void set_kusudama_open_cone(int32_t p_bone, int32_t p_index,
			Vector3 p_center, float p_radius);
	Vector3 get_kusudama_open_cone_center(int32_t p_constraint_index, int32_t p_index) const;
	float get_kusudama_open_cone_radius(int32_t p_constraint_index, int32_t p_index) const;
	int32_t get_kusudama_open_cone_count(int32_t p_constraint_index) const;
	int32_t get_bone_count() const;
	void set_kusudama_twist_from_to(int32_t p_index, float from, float to);
	void set_kusudama_open_cone_count(int32_t p_constraint_index, int32_t p_count);
	void set_kusudama_open_cone_center(int32_t p_constraint_index, int32_t p_index, Vector3 p_center);
	void set_kusudama_open_cone_radius(int32_t p_constraint_index, int32_t p_index, float p_radius);
	ManyBoneIK3D();
	~ManyBoneIK3D();
	void set_dirty();
};

void ManyBoneIK3D::set_total_effector_count(int32_t p_value) {
	int32_t old_count = pins.size();
	pin_count = p_value;
	pins.resize(p_value);
	for (int32_t pin_i = p_value; pin_i-- > old_count;) {
		pins.write[pin_i].instantiate();
	}
	set_dirty();
}

int32_t ManyBoneIK3D::get_effector_count() const {
	return pin_count;
}

void ManyBoneIK3D::set_effector_target_node_path(int32_t p_pin_index, const NodePath &p_target_node) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	if (effector_template.is_null()) {
		effector_template.instantiate();
		pins.write[p_pin_index] = effector_template;
	}
	effector_template->set_target_node(p_target_node);
	set_dirty();
}

NodePath ManyBoneIK3D::get_effector_target_node_path(int32_t p_pin_index) {
	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), NodePath());
	const Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	return effector_template->get_target_node();
}

Vector<Ref<IKEffectorTemplate3D>> ManyBoneIK3D::_get_bone_effectors() const {
	return pins;
}

void ManyBoneIK3D::_remove_pin(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, pins.size());
	pins.remove_at(p_index);
	pin_count--;
	pins.resize(pin_count);
	set_dirty();
}

void ManyBoneIK3D::_update_ik_bones_transform() {
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

void ManyBoneIK3D::_update_skeleton_bones_transform() {
	for (int32_t bone_i = bone_list.size(); bone_i-- > 0;) {
		Ref<IKBone3D> bone = bone_list[bone_i];
		if (bone.is_null()) {
			continue;
		}
		if (bone->get_bone_id() == -1) {
			continue;
		}
		bone->set_skeleton_bone_pose(get_skeleton());
	}
	update_gizmos();
}

void ManyBoneIK3D::_get_property_list(List<PropertyInfo> *p_list) const {
	const Vector<Ref<IKBone3D>> ik_bones = get_bone_list();
	RBSet<StringName> existing_pins;
	for (int32_t pin_i = 0; pin_i < get_effector_count(); pin_i++) {
		const String bone_name = get_effector_bone_name(pin_i);
		existing_pins.insert(bone_name);
	}
	const uint32_t pin_usage = PROPERTY_USAGE_DEFAULT;
	p_list->push_back(
			PropertyInfo(Variant::INT, "pin_count",
					PROPERTY_HINT_RANGE, "0,65536,or_greater", pin_usage | PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_READ_ONLY,
					"Pins,pins/"));
	for (int pin_i = 0; pin_i < get_effector_count(); pin_i++) {
		PropertyInfo effector_name;
		effector_name.type = Variant::STRING_NAME;
		effector_name.name = "pins/" + itos(pin_i) + "/bone_name";
		effector_name.usage = pin_usage;
		if (get_skeleton()) {
			String names;
			for (int bone_i = 0; bone_i < get_skeleton()->get_bone_count(); bone_i++) {
				String name = get_skeleton()->get_bone_name(bone_i);
				StringName string_name = StringName(name);
				if (existing_pins.has(string_name)) {
					continue;
				}
				name += ",";
				names += name;
				existing_pins.insert(name);
			}
			effector_name.hint = PROPERTY_HINT_ENUM_SUGGESTION;
			effector_name.hint_string = names;
		} else {
			effector_name.hint = PROPERTY_HINT_NONE;
			effector_name.hint_string = "";
		}
		p_list->push_back(effector_name);
		p_list->push_back(
				PropertyInfo(Variant::NODE_PATH, "pins/" + itos(pin_i) + "/target_node", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D", pin_usage));
		p_list->push_back(
				PropertyInfo(Variant::BOOL, "pins/" + itos(pin_i) + "/target_static", PROPERTY_HINT_NONE, "", pin_usage));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/passthrough_factor", PROPERTY_HINT_RANGE, "0,1,0.1,or_greater", pin_usage));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "pins/" + itos(pin_i) + "/weight", PROPERTY_HINT_RANGE, "0,1,0.1,or_greater", pin_usage));
		p_list->push_back(
				PropertyInfo(Variant::VECTOR3, "pins/" + itos(pin_i) + "/direction_priorities", PROPERTY_HINT_RANGE, "0,1,0.1,or_greater", pin_usage));
	}
	uint32_t constraint_usage = PROPERTY_USAGE_DEFAULT;
	p_list->push_back(
			PropertyInfo(Variant::INT, "constraint_count",
					PROPERTY_HINT_RANGE, "0,256,or_greater", constraint_usage | PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_READ_ONLY,
					"Kusudama Constraints,constraints/"));
	RBSet<String> existing_constraints;
	for (int constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		PropertyInfo bone_name;
		bone_name.type = Variant::STRING_NAME;
		bone_name.usage = constraint_usage;
		bone_name.name = "constraints/" + itos(constraint_i) + "/bone_name";
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
				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/twist_start", PROPERTY_HINT_RANGE, "-359.9,359.9,0.1,radians,exp", constraint_usage));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/twist_end", PROPERTY_HINT_RANGE, "-359.9,359.9,0.1,radians,exp", constraint_usage));
		p_list->push_back(
				PropertyInfo(Variant::INT, "constraints/" + itos(constraint_i) + "/kusudama_open_cone_count", PROPERTY_HINT_RANGE, "0,10,1", constraint_usage | PROPERTY_USAGE_ARRAY | PROPERTY_USAGE_READ_ONLY,
						"Limit Cones,constraints/" + itos(constraint_i) + "/kusudama_open_cone/"));
		for (int cone_i = 0; cone_i < get_kusudama_open_cone_count(constraint_i); cone_i++) {
			p_list->push_back(
					PropertyInfo(Variant::VECTOR3, "constraints/" + itos(constraint_i) + "/kusudama_open_cone/" + itos(cone_i) + "/center", PROPERTY_HINT_RANGE, "-1,1,0.1,exp", constraint_usage));

			p_list->push_back(
					PropertyInfo(Variant::FLOAT, "constraints/" + itos(constraint_i) + "/kusudama_open_cone/" + itos(cone_i) + "/radius", PROPERTY_HINT_RANGE, "0,180,0.1,radians,exp", constraint_usage));
		}
		p_list->push_back(
				PropertyInfo(Variant::TRANSFORM3D, "constraints/" + itos(constraint_i) + "/kusudama_twist", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
		p_list->push_back(
				PropertyInfo(Variant::TRANSFORM3D, "constraints/" + itos(constraint_i) + "/kusudama_orientation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
		p_list->push_back(
				PropertyInfo(Variant::TRANSFORM3D, "constraints/" + itos(constraint_i) + "/bone_direction", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
	}
}

bool ManyBoneIK3D::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "constraint_count") {
		r_ret = get_constraint_count();
		return true;
	} else if (name == "pin_count") {
		r_ret = get_effector_count();
		return true;
	} else if (name == "bone_count") {
		r_ret = get_bone_count();
		return true;
	} else if (name.begins_with("pins/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, pins.size(), false);
		Ref<IKEffectorTemplate3D> effector_template = pins[index];
		ERR_FAIL_NULL_V(effector_template, false);
		if (what == "bone_name") {
			r_ret = effector_template->get_name();
			return true;
		} else if (what == "target_node") {
			r_ret = effector_template->get_target_node();
			return true;
		} else if (what == "target_static") {
			r_ret = effector_template->get_target_node().is_empty();
			return true;
		} else if (what == "passthrough_factor") {
			r_ret = get_pin_passthrough_factor(index);
			return true;
		} else if (what == "weight") {
			r_ret = get_pin_weight(index);
			return true;
		} else if (what == "direction_priorities") {
			r_ret = get_pin_direction_priorities(index);
			return true;
		}
	} else if (name.begins_with("constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, constraint_count, false);
		String begins = "constraints/" + itos(index) + "/kusudama_open_cone";
		if (what == "bone_name") {
			ERR_FAIL_INDEX_V(index, constraint_names.size(), false);
			r_ret = constraint_names[index];
			return true;
		} else if (what == "twist_start") {
			r_ret = get_joint_twist(index).x;
			return true;
		} else if (what == "twist_end") {
			r_ret = get_joint_twist(index).y;
			return true;
		} else if (what == "kusudama_open_cone_count") {
			r_ret = get_kusudama_open_cone_count(index);
			return true;
		} else if (name.begins_with(begins)) {
			int32_t cone_index = name.get_slicec('/', 3).to_int();
			String cone_what = name.get_slicec('/', 4);
			if (cone_what == "center") {
				Vector3 center = get_kusudama_open_cone_center(index, cone_index);
				r_ret = center;
				return true;
			} else if (cone_what == "radius") {
				r_ret = get_kusudama_open_cone_radius(index, cone_index);
				return true;
			}
		} else if (what == "bone_direction") {
			r_ret = get_direction_transform_of_bone(index);
			return true;
		} else if (what == "kusudama_orientation") {
			r_ret = get_orientation_transform_of_constraint(index);
			return true;
		} else if (what == "kusudama_twist") {
			r_ret = get_twist_transform_of_constraint(index);
			return true;
		}
	}
	return false;
}

bool ManyBoneIK3D::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "constraint_count") {
		_set_constraint_count(p_value);
		return true;
	} else if (name == "pin_count") {
		set_total_effector_count(p_value);
		return true;
	} else if (name.begins_with("pins/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		if (index >= pins.size()) {
			set_total_effector_count(constraint_count);
		}
		if (what == "bone_name") {
			set_effector_bone_name(index, p_value);
			return true;
		} else if (what == "target_node") {
			set_effector_target_node_path(index, p_value);
			return true;
		} else if (what == "target_static") {
			if (p_value) {
				set_effector_target_node_path(index, NodePath());
			}
			return true;
		} else if (what == "passthrough_factor") {
			set_pin_passthrough_factor(index, p_value);
			return true;
		} else if (what == "weight") {
			set_pin_weight(index, p_value);
			return true;
		} else if (what == "direction_priorities") {
			set_pin_direction_priorities(index, p_value);
			return true;
		}
	} else if (name.begins_with("constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		String begins = "constraints/" + itos(index) + "/kusudama_open_cone/";
		if (index >= constraint_names.size()) {
			_set_constraint_count(constraint_count);
		}
		if (what == "bone_name") {
			set_constraint_name_at_index(index, p_value);
			return true;
		} else if (what == "twist_from") {
			Vector2 twist_from = get_joint_twist(index);
			set_joint_twist(index, Vector2(p_value, twist_from.y));
			return true;
		} else if (what == "twist_range") {
			Vector2 twist_range = get_joint_twist(index);
			set_joint_twist(index, Vector2(twist_range.x, p_value));
			return true;
		} else if (what == "kusudama_open_cone_count") {
			set_kusudama_open_cone_count(index, p_value);
			return true;
		} else if (name.begins_with(begins)) {
			int cone_index = name.get_slicec('/', 3).to_int();
			String cone_what = name.get_slicec('/', 4);
			if (cone_what == "center") {
				set_kusudama_open_cone_center(index, cone_index, p_value);
				return true;
			} else if (cone_what == "radius") {
				set_kusudama_open_cone_radius(index, cone_index, p_value);
				return true;
			}
		} else if (what == "bone_direction") {
			set_direction_transform_of_bone(index, p_value);
			return true;
		} else if (what == "kusudama_orientation") {
			set_orientation_transform_of_constraint(index, p_value);
			return true;
		} else if (what == "kusudama_twist") {
			set_twist_transform_of_constraint(index, p_value);
			return true;
		}
	}

	return false;
}

void ManyBoneIK3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_constraint_name_at_index", "index", "name"), &ManyBoneIK3D::set_constraint_name_at_index);
	ClassDB::bind_method(D_METHOD("set_total_effector_count", "count"), &ManyBoneIK3D::set_total_effector_count);
	ClassDB::bind_method(D_METHOD("get_twist_transform_of_constraint", "index"), &ManyBoneIK3D::get_twist_transform_of_constraint);
	ClassDB::bind_method(D_METHOD("set_twist_transform_of_constraint", "index", "transform"), &ManyBoneIK3D::set_twist_transform_of_constraint);
	ClassDB::bind_method(D_METHOD("get_orientation_transform_of_constraint", "index"), &ManyBoneIK3D::get_orientation_transform_of_constraint);
	ClassDB::bind_method(D_METHOD("set_orientation_transform_of_constraint", "index", "transform"), &ManyBoneIK3D::set_orientation_transform_of_constraint);
	ClassDB::bind_method(D_METHOD("get_direction_transform_of_bone", "index"), &ManyBoneIK3D::get_direction_transform_of_bone);
	ClassDB::bind_method(D_METHOD("set_direction_transform_of_bone", "index", "transform"), &ManyBoneIK3D::set_direction_transform_of_bone);
	ClassDB::bind_method(D_METHOD("remove_constraint_at_index", "index"), &ManyBoneIK3D::remove_constraint_at_index);
	ClassDB::bind_method(D_METHOD("register_skeleton"), &ManyBoneIK3D::register_skeleton);
	ClassDB::bind_method(D_METHOD("reset_constraints"), &ManyBoneIK3D::reset_constraints);
	ClassDB::bind_method(D_METHOD("set_dirty"), &ManyBoneIK3D::set_dirty);
	ClassDB::bind_method(D_METHOD("set_kusudama_open_cone_radius", "index", "cone_index", "radius"), &ManyBoneIK3D::set_kusudama_open_cone_radius);
	ClassDB::bind_method(D_METHOD("get_kusudama_open_cone_radius", "index", "cone_index"), &ManyBoneIK3D::get_kusudama_open_cone_radius);
	ClassDB::bind_method(D_METHOD("set_kusudama_open_cone_center", "index", "cone_index", "center"), &ManyBoneIK3D::set_kusudama_open_cone_center);
	ClassDB::bind_method(D_METHOD("get_kusudama_open_cone_center", "index", "cone_index"), &ManyBoneIK3D::get_kusudama_open_cone_center);
	ClassDB::bind_method(D_METHOD("set_kusudama_open_cone_count", "index", "count"), &ManyBoneIK3D::set_kusudama_open_cone_count);
	ClassDB::bind_method(D_METHOD("get_kusudama_open_cone_count", "index"), &ManyBoneIK3D::get_kusudama_open_cone_count);
	ClassDB::bind_method(D_METHOD("set_joint_twist", "index", "limit"), &ManyBoneIK3D::set_joint_twist);
	ClassDB::bind_method(D_METHOD("get_joint_twist", "index"), &ManyBoneIK3D::get_joint_twist);
	ClassDB::bind_method(D_METHOD("set_pin_passthrough_factor", "index", "falloff"), &ManyBoneIK3D::set_pin_passthrough_factor);
	ClassDB::bind_method(D_METHOD("get_pin_passthrough_factor", "index"), &ManyBoneIK3D::get_pin_passthrough_factor);
	ClassDB::bind_method(D_METHOD("get_pin_count"), &ManyBoneIK3D::get_effector_count);
	ClassDB::bind_method(D_METHOD("get_effector_bone_name", "index"), &ManyBoneIK3D::get_effector_bone_name);
	ClassDB::bind_method(D_METHOD("get_pin_direction_priorities", "index"), &ManyBoneIK3D::get_pin_direction_priorities);
	ClassDB::bind_method(D_METHOD("set_pin_direction_priorities", "index", "priority"), &ManyBoneIK3D::set_pin_direction_priorities);
	ClassDB::bind_method(D_METHOD("get_effector_pin_node_path", "index"), &ManyBoneIK3D::get_effector_pin_node_path);
	ClassDB::bind_method(D_METHOD("set_effector_pin_node_path", "index", "nodepath"), &ManyBoneIK3D::set_effector_pin_node_path);
	ClassDB::bind_method(D_METHOD("set_pin_weight", "index", "weight"), &ManyBoneIK3D::set_pin_weight);
	ClassDB::bind_method(D_METHOD("get_pin_weight", "index"), &ManyBoneIK3D::get_pin_weight);
	ClassDB::bind_method(D_METHOD("get_pin_enabled", "index"), &ManyBoneIK3D::get_pin_enabled);
	ClassDB::bind_method(D_METHOD("get_constraint_name", "index"), &ManyBoneIK3D::get_constraint_name);
	ClassDB::bind_method(D_METHOD("get_iterations_per_frame"), &ManyBoneIK3D::get_iterations_per_frame);
	ClassDB::bind_method(D_METHOD("set_iterations_per_frame", "count"), &ManyBoneIK3D::set_iterations_per_frame);
	ClassDB::bind_method(D_METHOD("find_constraint", "name"), &ManyBoneIK3D::find_constraint);
	ClassDB::bind_method(D_METHOD("find_pin", "name"), &ManyBoneIK3D::find_pin);
	ClassDB::bind_method(D_METHOD("get_constraint_count"), &ManyBoneIK3D::get_constraint_count);
	ClassDB::bind_method(D_METHOD("set_constraint_count", "count"), &ManyBoneIK3D::_set_constraint_count);
	ClassDB::bind_method(D_METHOD("queue_print_skeleton"), &ManyBoneIK3D::queue_print_skeleton);
	ClassDB::bind_method(D_METHOD("get_default_damp"), &ManyBoneIK3D::get_default_damp);
	ClassDB::bind_method(D_METHOD("set_default_damp", "damp"), &ManyBoneIK3D::set_default_damp);
	ClassDB::bind_method(D_METHOD("get_bone_count"), &ManyBoneIK3D::get_bone_count);
	ClassDB::bind_method(D_METHOD("set_constraint_mode", "enabled"), &ManyBoneIK3D::set_constraint_mode);
	ClassDB::bind_method(D_METHOD("get_constraint_mode"), &ManyBoneIK3D::get_constraint_mode);
	ClassDB::bind_method(D_METHOD("set_ui_selected_bone", "bone"), &ManyBoneIK3D::set_ui_selected_bone);
	ClassDB::bind_method(D_METHOD("get_ui_selected_bone"), &ManyBoneIK3D::get_ui_selected_bone);
	ClassDB::bind_method(D_METHOD("set_stabilization_passes", "passes"), &ManyBoneIK3D::set_stabilization_passes);
	ClassDB::bind_method(D_METHOD("get_stabilization_passes"), &ManyBoneIK3D::get_stabilization_passes);
	ClassDB::bind_method(D_METHOD("set_effector_bone_name", "index", "name"), &ManyBoneIK3D::set_effector_bone_name);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "iterations_per_frame", PROPERTY_HINT_RANGE, "1,150,1,or_greater"), "set_iterations_per_frame", "get_iterations_per_frame");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "default_damp", PROPERTY_HINT_RANGE, "0.01,180.0,0.1,radians,exp", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), "set_default_damp", "get_default_damp");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "constraint_mode"), "set_constraint_mode", "get_constraint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ui_selected_bone", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_ui_selected_bone", "get_ui_selected_bone");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stabilization_passes"), "set_stabilization_passes", "get_stabilization_passes");
}

ManyBoneIK3D::ManyBoneIK3D() {
}

ManyBoneIK3D::~ManyBoneIK3D() {
}

void ManyBoneIK3D::queue_print_skeleton() {
	queue_debug_skeleton = true;
}

float ManyBoneIK3D::get_pin_passthrough_factor(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), 0.0f);
	const Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	return effector_template->get_passthrough_factor();
}

void ManyBoneIK3D::set_pin_passthrough_factor(int32_t p_effector_index, const float p_passthrough_factor) {
	ERR_FAIL_INDEX(p_effector_index, pins.size());
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	ERR_FAIL_NULL(effector_template);
	effector_template->set_passthrough_factor(p_passthrough_factor);
	set_dirty();
}

void ManyBoneIK3D::_set_constraint_count(int32_t p_count) {
	int32_t old_count = constraint_names.size();
	constraint_count = p_count;
	constraint_names.resize(p_count);
	joint_twist.resize(p_count);
	kusudama_open_cone_count.resize(p_count);
	kusudama_open_cones.resize(p_count);
	for (int32_t constraint_i = p_count; constraint_i-- > old_count;) {
		constraint_names.write[constraint_i] = String();
		kusudama_open_cone_count.write[constraint_i] = 0;
		kusudama_open_cones.write[constraint_i].resize(1);
		kusudama_open_cones.write[constraint_i].write[0] = Vector4(0, 1, 0, 0.01745f);
		joint_twist.write[constraint_i] = Vector2(0, 0.01745f);
	}
	set_dirty();
	notify_property_list_changed();
}

int32_t ManyBoneIK3D::get_constraint_count() const {
	return constraint_count;
}

inline StringName ManyBoneIK3D::get_constraint_name(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), StringName());
	return constraint_names[p_index];
}

Vector2 ManyBoneIK3D::get_joint_twist(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, joint_twist.size(), Vector2());
	return joint_twist[p_index];
}

void ManyBoneIK3D::set_joint_twist(int32_t p_index, Vector2 p_to) {
	ERR_FAIL_INDEX(p_index, constraint_count);
	joint_twist.write[p_index] = p_to;
	set_dirty();
}

int32_t ManyBoneIK3D::find_effector_id(StringName p_bone_name) {
	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		if (constraint_names[constraint_i] == p_bone_name) {
			return constraint_i;
		}
	}
	return -1;
}

void ManyBoneIK3D::set_kusudama_open_cone(int32_t p_constraint_index, int32_t p_index,
		Vector3 p_center, float p_radius) {
	ERR_FAIL_INDEX(p_constraint_index, kusudama_open_cones.size());
	Vector<Vector4> cones = kusudama_open_cones.write[p_constraint_index];
	if (Math::is_zero_approx(p_center.length_squared())) {
		p_center = Vector3(0.0f, 1.0f, 0.0f);
	}
	Vector3 center = p_center.normalized();
	Vector4 cone;
	cone.x = center.x;
	cone.y = center.y;
	cone.z = center.z;
	cone.w = p_radius;
	cones.write[p_index] = cone;
	kusudama_open_cones.write[p_constraint_index] = cones;
	set_dirty();
}

float ManyBoneIK3D::get_kusudama_open_cone_radius(int32_t p_constraint_index, int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_constraint_index, kusudama_open_cones.size(), Math_TAU);
	ERR_FAIL_INDEX_V(p_index, kusudama_open_cones[p_constraint_index].size(), Math_TAU);
	return kusudama_open_cones[p_constraint_index][p_index].w;
}

int32_t ManyBoneIK3D::get_kusudama_open_cone_count(int32_t p_constraint_index) const {
	ERR_FAIL_INDEX_V(p_constraint_index, kusudama_open_cone_count.size(), 0);
	return kusudama_open_cone_count[p_constraint_index];
}

void ManyBoneIK3D::set_kusudama_open_cone_count(int32_t p_constraint_index, int32_t p_count) {
	ERR_FAIL_INDEX(p_constraint_index, kusudama_open_cone_count.size());
	ERR_FAIL_INDEX(p_constraint_index, kusudama_open_cones.size());
	int32_t old_cone_count = kusudama_open_cones[p_constraint_index].size();
	kusudama_open_cone_count.write[p_constraint_index] = p_count;
	Vector<Vector4> &cones = kusudama_open_cones.write[p_constraint_index];
	cones.resize(p_count);
	String bone_name = get_constraint_name(p_constraint_index);
	Transform3D bone_transform = get_direction_transform_of_bone(p_constraint_index);
	Vector3 forward_axis = -bone_transform.basis.get_column(Vector3::AXIS_Y).normalized();
	for (int32_t cone_i = p_count; cone_i-- > old_cone_count;) {
		Vector4 &cone = cones.write[cone_i];
		cone.x = forward_axis.x;
		cone.y = forward_axis.y;
		cone.z = forward_axis.z;
		cone.w = Math::deg_to_rad(0.0f);
	}
	set_dirty();
	notify_property_list_changed();
}

real_t ManyBoneIK3D::get_default_damp() const {
	return default_damp;
}

void ManyBoneIK3D::set_default_damp(float p_default_damp) {
	default_damp = p_default_damp;
	set_dirty();
}

StringName ManyBoneIK3D::get_effector_bone_name(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), "");
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	return effector_template->get_name();
}

void ManyBoneIK3D::set_kusudama_open_cone_radius(int32_t p_effector_index, int32_t p_index, float p_radius) {
	ERR_FAIL_INDEX(p_effector_index, kusudama_open_cone_count.size());
	ERR_FAIL_INDEX(p_effector_index, kusudama_open_cones.size());
	ERR_FAIL_INDEX(p_index, kusudama_open_cone_count[p_effector_index]);
	ERR_FAIL_INDEX(p_index, kusudama_open_cones[p_effector_index].size());
	Vector4 &cone = kusudama_open_cones.write[p_effector_index].write[p_index];
	cone.w = p_radius;
	set_dirty();
}

void ManyBoneIK3D::set_kusudama_open_cone_center(int32_t p_effector_index, int32_t p_index, Vector3 p_center) {
	ERR_FAIL_INDEX(p_effector_index, kusudama_open_cones.size());
	ERR_FAIL_INDEX(p_index, kusudama_open_cones[p_effector_index].size());
	Vector4 &cone = kusudama_open_cones.write[p_effector_index].write[p_index];
	Basis basis;
	basis.set_column(0, Vector3(1, 0, 0));
	basis.set_column(1, Vector3(0, 0, -1));
	basis.set_column(2, Vector3(0, 1, 0));
	if (Math::is_zero_approx(p_center.length_squared())) {
		cone.x = 0;
		cone.y = 0;
		cone.z = 1;
	} else {
		p_center = basis.xform(p_center);
		cone.x = p_center.x;
		cone.y = p_center.y;
		cone.z = p_center.z;
	}
	set_dirty();
}

Vector3 ManyBoneIK3D::get_kusudama_open_cone_center(int32_t p_constraint_index, int32_t p_index) const {
	if (unlikely((p_constraint_index) < 0 || (p_constraint_index) >= (kusudama_open_cones.size()))) {
		ERR_PRINT_ONCE("Can't get limit cone center.");
		return Vector3(0.0, 0.0, 1.0);
	}
	if (unlikely((p_index) < 0 || (p_index) >= (kusudama_open_cones[p_constraint_index].size()))) {
		ERR_PRINT_ONCE("Can't get limit cone center.");
		return Vector3(0.0, 0.0, 1.0);
	}
	const Vector4 &cone = kusudama_open_cones[p_constraint_index][p_index];
	Vector3 ret;
	ret.x = cone.x;
	ret.y = cone.y;
	ret.z = cone.z;
	Basis basis;
	basis.set_column(0, Vector3(1, 0, 0));
	basis.set_column(1, Vector3(0, 0, -1));
	basis.set_column(2, Vector3(0, 1, 0));
	return basis.xform_inv(ret);
}

void ManyBoneIK3D::set_constraint_name_at_index(int32_t p_index, String p_name) {
	ERR_FAIL_INDEX(p_index, constraint_names.size());
	constraint_names.write[p_index] = p_name;
	set_dirty();
}

Vector<Ref<IKBoneSegment3D>> ManyBoneIK3D::get_segmented_skeletons() {
	return segmented_skeletons;
}

float ManyBoneIK3D::get_iterations_per_frame() const {
	return iterations_per_frame;
}

void ManyBoneIK3D::set_iterations_per_frame(const float &p_iterations_per_frame) {
	iterations_per_frame = p_iterations_per_frame;
}

void ManyBoneIK3D::set_effector_pin_node_path(int32_t p_effector_index, NodePath p_node_path) {
	ERR_FAIL_INDEX(p_effector_index, pins.size());
	Node *node = get_node_or_null(p_node_path);
	if (!node) {
		return;
	}
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	effector_template->set_target_node(p_node_path);
}

NodePath ManyBoneIK3D::get_effector_pin_node_path(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), NodePath());
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	return effector_template->get_target_node();
}

void ManyBoneIK3D::_process_modification() {
	if (!get_skeleton()) {
		return;
	}
	if (get_effector_count() == 0) {
		return;
	}
	if (!segmented_skeletons.size()) {
		set_dirty();
	}
	if (is_dirty) {
		is_dirty = false;
		_bone_list_changed();
	}
	if (bone_list.size()) {
		Ref<IKNode3D> root_ik_bone = bone_list.write[0]->get_ik_transform();
		if (root_ik_bone.is_null()) {
			return;
		}
		Skeleton3D *skeleton = get_skeleton();
		godot_skeleton_transform.instantiate();
		godot_skeleton_transform->set_transform(skeleton->get_transform());
		godot_skeleton_transform_inverse = skeleton->get_transform().affine_inverse();
	}
	bool has_pins = false;
	for (Ref<IKEffectorTemplate3D> pin : pins) {
		if (pin.is_valid() && !pin->get_name().is_empty()) {
			has_pins = true;
			break;
		}
	}
	if (!has_pins) {
		return;
	}
	if (!is_enabled()) {
		return;
	}
	if (!is_visible()) {
		return;
	}
	for (int32_t i = 0; i < get_iterations_per_frame(); i++) {
		for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
			if (segmented_skeleton.is_null()) {
				continue;
			}
			segmented_skeleton->segment_solver(bone_damp, get_default_damp(), get_constraint_mode(), i, get_iterations_per_frame());
		}
	}
	_update_skeleton_bones_transform();
}

real_t ManyBoneIK3D::get_pin_weight(int32_t p_pin_index) const {
	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), 0.0);
	const Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	return effector_template->get_weight();
}

void ManyBoneIK3D::set_pin_weight(int32_t p_pin_index, const real_t &p_weight) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	if (effector_template.is_null()) {
		effector_template.instantiate();
		pins.write[p_pin_index] = effector_template;
	}
	effector_template->set_weight(p_weight);
	set_dirty();
}

Vector3 ManyBoneIK3D::get_pin_direction_priorities(int32_t p_pin_index) const {
	ERR_FAIL_INDEX_V(p_pin_index, pins.size(), Vector3(0, 0, 0));
	const Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	return effector_template->get_direction_priorities();
}

void ManyBoneIK3D::set_pin_direction_priorities(int32_t p_pin_index, const Vector3 &p_priority_direction) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	if (effector_template.is_null()) {
		effector_template.instantiate();
		pins.write[p_pin_index] = effector_template;
	}
	effector_template->set_direction_priorities(p_priority_direction);
	set_dirty();
}

void ManyBoneIK3D::set_dirty() {
	is_dirty = true;
}

int32_t ManyBoneIK3D::find_constraint(String p_string) const {
	for (int32_t constraint_i = 0; constraint_i < constraint_count; constraint_i++) {
		if (get_constraint_name(constraint_i) == p_string) {
			return constraint_i;
		}
	}
	return -1;
}

void ManyBoneIK3D::remove_constraint_at_index(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, constraint_count);

	constraint_names.remove_at(p_index);
	kusudama_open_cone_count.remove_at(p_index);
	kusudama_open_cones.remove_at(p_index);
	joint_twist.remove_at(p_index);

	constraint_count--;

	set_dirty();
}

void ManyBoneIK3D::_set_bone_count(int32_t p_count) {
	bone_damp.resize(p_count);
	for (int32_t bone_i = p_count; bone_i-- > bone_count;) {
		bone_damp.write[bone_i] = get_default_damp();
	}
	bone_count = p_count;
	set_dirty();
	notify_property_list_changed();
}

int32_t ManyBoneIK3D::get_bone_count() const {
	return bone_count;
}

Vector<Ref<IKBone3D>> ManyBoneIK3D::get_bone_list() const {
	return bone_list;
}

void ManyBoneIK3D::set_direction_transform_of_bone(int32_t p_index, Transform3D p_transform) {
	ERR_FAIL_INDEX(p_index, constraint_names.size());
	if (!get_skeleton()) {
		return;
	}
	String bone_name = constraint_names[p_index];
	int32_t bone_index = get_skeleton()->find_bone(bone_name);
	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
		if (segmented_skeleton.is_null()) {
			continue;
		}
		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(bone_index);
		if (ik_bone.is_null() || ik_bone->get_constraint().is_null()) {
			continue;
		}
		if (ik_bone->get_bone_direction_transform().is_null()) {
			continue;
		}
		ik_bone->get_bone_direction_transform()->set_transform(p_transform);
		break;
	}
}

Transform3D ManyBoneIK3D::get_direction_transform_of_bone(int32_t p_index) const {
	if (p_index < 0 || p_index >= constraint_names.size() || get_skeleton() == nullptr) {
		return Transform3D();
	}

	String bone_name = constraint_names[p_index];
	int32_t bone_index = get_skeleton()->find_bone(bone_name);
	for (const Ref<IKBoneSegment3D> &segmented_skeleton : segmented_skeletons) {
		if (segmented_skeleton.is_null()) {
			continue;
		}
		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(bone_index);
		if (ik_bone.is_null() || ik_bone->get_constraint().is_null()) {
			continue;
		}
		return ik_bone->get_bone_direction_transform()->get_transform();
	}
	return Transform3D();
}

Transform3D ManyBoneIK3D::get_orientation_transform_of_constraint(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), Transform3D());
	String bone_name = constraint_names[p_index];
	if (!segmented_skeletons.size()) {
		return Transform3D();
	}
	if (!get_skeleton()) {
		return Transform3D();
	}
	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
		if (segmented_skeleton.is_null()) {
			continue;
		}
		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
		if (ik_bone.is_null()) {
			continue;
		}
		if (ik_bone->get_constraint().is_null()) {
			continue;
		}
		return ik_bone->get_constraint_orientation_transform()->get_transform();
	}
	return Transform3D();
}

void ManyBoneIK3D::set_orientation_transform_of_constraint(int32_t p_index, Transform3D p_transform) {
	ERR_FAIL_INDEX(p_index, constraint_names.size());
	String bone_name = constraint_names[p_index];
	if (!get_skeleton()) {
		return;
	}
	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
		if (segmented_skeleton.is_null()) {
			continue;
		}
		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
		if (ik_bone.is_null()) {
			continue;
		}
		if (ik_bone->get_constraint().is_null()) {
			continue;
		}
		ik_bone->get_constraint_orientation_transform()->set_transform(p_transform);
		break;
	}
}

Transform3D ManyBoneIK3D::get_twist_transform_of_constraint(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, constraint_names.size(), Transform3D());
	String bone_name = constraint_names[p_index];
	if (!segmented_skeletons.size()) {
		return Transform3D();
	}
	if (!get_skeleton()) {
		return Transform3D();
	}
	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
		if (segmented_skeleton.is_null()) {
			continue;
		}
		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
		if (ik_bone.is_null()) {
			continue;
		}
		if (ik_bone->get_constraint().is_null()) {
			continue;
		}
		return ik_bone->get_constraint_twist_transform()->get_transform();
	}
	return Transform3D();
}

void ManyBoneIK3D::set_twist_transform_of_constraint(int32_t p_index, Transform3D p_transform) {
	ERR_FAIL_INDEX(p_index, constraint_names.size());
	String bone_name = constraint_names[p_index];
	if (!get_skeleton()) {
		return;
	}
	for (Ref<IKBoneSegment3D> segmented_skeleton : segmented_skeletons) {
		if (segmented_skeleton.is_null()) {
			continue;
		}
		Ref<IKBone3D> ik_bone = segmented_skeleton->get_ik_bone(get_skeleton()->find_bone(bone_name));
		if (ik_bone.is_null()) {
			continue;
		}
		if (ik_bone->get_constraint().is_null()) {
			continue;
		}
		ik_bone->get_constraint_twist_transform()->set_transform(p_transform);
		break;
	}
}

bool ManyBoneIK3D::get_pin_enabled(int32_t p_effector_index) const {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), false);
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	if (effector_template->get_target_node().is_empty()) {
		return true;
	}
	return !effector_template->get_target_node().is_empty();
}

void ManyBoneIK3D::register_skeleton() {
	if (!get_effector_count() && !get_constraint_count()) {
		reset_constraints();
	}
	set_dirty();
}

void ManyBoneIK3D::reset_constraints() {
	Skeleton3D *skeleton = get_skeleton();
	if (skeleton) {
		int32_t saved_pin_count = get_effector_count();
		set_total_effector_count(0);
		set_total_effector_count(saved_pin_count);
		int32_t saved_constraint_count = constraint_names.size();
		_set_constraint_count(0);
		_set_constraint_count(saved_constraint_count);
		_set_bone_count(0);
		_set_bone_count(saved_constraint_count);
	}
	set_dirty();
}

bool ManyBoneIK3D::get_constraint_mode() const {
	return is_constraint_mode;
}

void ManyBoneIK3D::set_constraint_mode(bool p_enabled) {
	is_constraint_mode = p_enabled;
}

int32_t ManyBoneIK3D::get_ui_selected_bone() const {
	return ui_selected_bone;
}

void ManyBoneIK3D::set_ui_selected_bone(int32_t p_ui_selected_bone) {
	ui_selected_bone = p_ui_selected_bone;
}

void ManyBoneIK3D::set_stabilization_passes(int32_t p_passes) {
	stabilize_passes = p_passes;
	set_dirty();
}

int32_t ManyBoneIK3D::get_stabilization_passes() {
	return stabilize_passes;
}

Transform3D ManyBoneIK3D::get_godot_skeleton_transform_inverse() {
	return godot_skeleton_transform_inverse;
}

Ref<IKNode3D> ManyBoneIK3D::get_godot_skeleton_transform() {
	return godot_skeleton_transform;
}

void ManyBoneIK3D::add_constraint() {
	int32_t old_count = constraint_count;
	_set_constraint_count(constraint_count + 1);
	constraint_names.write[old_count] = String();
	kusudama_open_cone_count.write[old_count] = 0;
	kusudama_open_cones.write[old_count].resize(1);
	kusudama_open_cones.write[old_count].write[0] = Vector4(0, 1, 0, Math_PI);
	joint_twist.write[old_count] = Vector2(0, Math_PI);
	set_dirty();
}

int32_t ManyBoneIK3D::find_pin(String p_string) const {
	for (int32_t pin_i = 0; pin_i < pin_count; pin_i++) {
		if (get_effector_bone_name(pin_i) == p_string) {
			return pin_i;
		}
	}
	return -1;
}

bool ManyBoneIK3D::get_effector_target_fixed(int32_t p_effector_index) {
	ERR_FAIL_INDEX_V(p_effector_index, pins.size(), false);
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	return get_effector_pin_node_path(p_effector_index).is_empty();
}

void ManyBoneIK3D::set_effector_target_fixed(int32_t p_effector_index, bool p_force_ignore) {
	ERR_FAIL_INDEX(p_effector_index, pins.size());
	if (!p_force_ignore) {
		return;
	}
	Ref<IKEffectorTemplate3D> effector_template = pins[p_effector_index];
	effector_template->set_target_node(NodePath());
	set_dirty();
}

void ManyBoneIK3D::_bone_list_changed() {
	Skeleton3D *skeleton = get_skeleton();
	Vector<int32_t> roots = skeleton->get_parentless_bones();
	if (roots.is_empty()) {
		return;
	}
	bone_list.clear();
	segmented_skeletons.clear();
	for (BoneId root_bone_index : roots) {
		String parentless_bone = skeleton->get_bone_name(root_bone_index);
		Ref<IKBoneSegment3D> segmented_skeleton = Ref<IKBoneSegment3D>(memnew(IKBoneSegment3D(skeleton, parentless_bone, pins, this, nullptr, root_bone_index, -1, stabilize_passes)));
		ik_origin.instantiate();
		segmented_skeleton->get_root()->get_ik_transform()->set_parent(ik_origin);
		segmented_skeleton->generate_default_segments(pins, root_bone_index, -1, this);
		Vector<Ref<IKBone3D>> new_bone_list;
		segmented_skeleton->create_bone_list(new_bone_list, true, queue_debug_skeleton);
		bone_list.append_array(new_bone_list);
		Vector<Vector<double>> weight_array;
		segmented_skeleton->update_pinned_list(weight_array);
		segmented_skeleton->recursive_create_headings_arrays_for(segmented_skeleton);
		segmented_skeletons.push_back(segmented_skeleton);
	}
	_update_ik_bones_transform();
	for (Ref<IKBone3D> &ik_bone_3d : bone_list) {
		ik_bone_3d->update_default_bone_direction_transform(skeleton);
	}
	for (int constraint_i = 0; constraint_i < constraint_count; ++constraint_i) {
		String bone = constraint_names[constraint_i];
		BoneId bone_id = skeleton->find_bone(bone);
		for (Ref<IKBone3D> &ik_bone_3d : bone_list) {
			if (ik_bone_3d->get_bone_id() != bone_id) {
				continue;
			}
			Ref<IKKusudama3D> constraint;
			constraint.instantiate();
			constraint->enable_orientational_limits();

			int32_t cone_count = kusudama_open_cone_count[constraint_i];
			const Vector<Vector4> &cones = kusudama_open_cones[constraint_i];
			for (int32_t cone_i = 0; cone_i < cone_count; ++cone_i) {
				const Vector4 &cone = cones[cone_i];
				Ref<IKOpenCone3D> new_cone;
				new_cone.instantiate();
				new_cone->set_attached_to(constraint);
				new_cone->set_tangent_circle_center_next_1(Vector3(0.0f, -1.0f, 0.0f));
				new_cone->set_tangent_circle_center_next_2(Vector3(0.0f, 1.0f, 0.0f));
				new_cone->set_radius(MAX(1.0e-38, cone.w));
				new_cone->set_control_point(Vector3(cone.x, cone.y, cone.z).normalized());
				constraint->add_open_cone(new_cone);
			}

			const Vector2 axial_limit = get_joint_twist(constraint_i);
			constraint->enable_axial_limits();
			constraint->set_axial_limits(axial_limit.x, axial_limit.y);
			ik_bone_3d->add_constraint(constraint);
			constraint->_update_constraint(ik_bone_3d->get_constraint_twist_transform());
			break;
		}
	}
	if (queue_debug_skeleton) {
		queue_debug_skeleton = false;
	}
}

void ManyBoneIK3D::_skeleton_changed(Skeleton3D *p_old, Skeleton3D *p_new) {
	if (p_old) {
		if (p_old->is_connected(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed))) {
			p_old->disconnect(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed));
		}
	}
	if (p_new) {
		if (!p_new->is_connected(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed))) {
			p_new->connect(SNAME("bone_list_changed"), callable_mp(this, &ManyBoneIK3D::_bone_list_changed));
		}
	}
	if (is_connected(SNAME("modification_processed"), callable_mp(this, &ManyBoneIK3D::_update_ik_bones_transform))) {
		disconnect(SNAME("modification_processed"), callable_mp(this, &ManyBoneIK3D::_update_ik_bones_transform));
	}
	connect(SNAME("modification_processed"), callable_mp(this, &ManyBoneIK3D::_update_ik_bones_transform));
	_bone_list_changed();
}

void ManyBoneIK3D::set_effector_bone_name(int32_t p_pin_index, const String &p_bone) {
	ERR_FAIL_INDEX(p_pin_index, pins.size());
	Ref<IKEffectorTemplate3D> effector_template = pins[p_pin_index];
	if (effector_template.is_null()) {
		effector_template.instantiate();
		pins.write[p_pin_index] = effector_template;
	}
	effector_template->set_name(p_bone);
	set_dirty();
}


class IKEffectorTemplate3D : public Resource {
	GDCLASS(IKEffectorTemplate3D, Resource);

	StringName root_bone;
	NodePath target_node;
	bool target_static = false;
	real_t passthrough_factor = 0.0f;
	real_t weight = 0.0f;
	Vector3 priority_direction = Vector3(0.2f, 0.0f, 0.2f); // Purported ideal values are 1.0 / 3.0 for one direction, 1.0 / 5.0 for two directions and 1.0 / 7.0 for three directions.
protected:
	static void _bind_methods();

public:
	String get_root_bone() const;
	void set_root_bone(String p_root_bone);
	NodePath get_target_node() const;
	void set_target_node(NodePath p_node_path);
	float get_passthrough_factor() const;
	void set_passthrough_factor(float p_passthrough_factor);
	real_t get_weight() const { return weight; }
	void set_weight(real_t p_weight) { weight = p_weight; }
	Vector3 get_direction_priorities() const { return priority_direction; }
	void set_direction_priorities(Vector3 p_priority_direction) { priority_direction = p_priority_direction; }

	IKEffectorTemplate3D();
};


void IKEffectorTemplate3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_root_bone"), &IKEffectorTemplate3D::get_root_bone);
	ClassDB::bind_method(D_METHOD("set_root_bone", "target_node"), &IKEffectorTemplate3D::set_root_bone);

	ClassDB::bind_method(D_METHOD("get_target_node"), &IKEffectorTemplate3D::get_target_node);
	ClassDB::bind_method(D_METHOD("set_target_node", "target_node"), &IKEffectorTemplate3D::set_target_node);

	ClassDB::bind_method(D_METHOD("get_passthrough_factor"), &IKEffectorTemplate3D::get_passthrough_factor);
	ClassDB::bind_method(D_METHOD("set_passthrough_factor", "passthrough_factor"), &IKEffectorTemplate3D::set_passthrough_factor);

	ClassDB::bind_method(D_METHOD("get_weight"), &IKEffectorTemplate3D::get_weight);
	ClassDB::bind_method(D_METHOD("set_weight", "weight"), &IKEffectorTemplate3D::set_weight);

	ClassDB::bind_method(D_METHOD("get_direction_priorities"), &IKEffectorTemplate3D::get_direction_priorities);
	ClassDB::bind_method(D_METHOD("set_direction_priorities", "direction_priorities"), &IKEffectorTemplate3D::set_direction_priorities);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "passthrough_factor"), "set_passthrough_factor", "get_passthrough_factor");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "weight"), "set_weight", "get_weight");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction_priorities"), "set_direction_priorities", "get_direction_priorities");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "root_bone"), "set_root_bone", "get_root_bone");
}

NodePath IKEffectorTemplate3D::get_target_node() const {
	return target_node;
}

void IKEffectorTemplate3D::set_target_node(NodePath p_node_path) {
	target_node = p_node_path;
}

float IKEffectorTemplate3D::get_passthrough_factor() const {
	return passthrough_factor;
}

void IKEffectorTemplate3D::set_passthrough_factor(float p_passthrough_factor) {
	passthrough_factor = p_passthrough_factor;
}

IKEffectorTemplate3D::IKEffectorTemplate3D() {
}

String IKEffectorTemplate3D::get_root_bone() const {
	return root_bone;
}

void IKEffectorTemplate3D::set_root_bone(String p_node_path) {
	root_bone = p_node_path;
}

class IKEffector3D : public Resource {
	GDCLASS(IKEffector3D, Resource);
	friend class IKBone3D;
	friend class IKBoneSegment3D;

	Ref<IKBone3D> for_bone;
	bool use_target_node_rotation = true;
	NodePath target_node_path;
	ObjectID target_node_cache;
	Node *target_node_reference = nullptr;
	bool target_static = false;
	Transform3D target_transform;

	Transform3D target_relative_to_skeleton_origin;
	int32_t num_headings = 7;
	// See IKEffectorTemplate to change the defaults.
	real_t weight = 0.0;
	real_t passthrough_factor = 0.0;
	PackedVector3Array target_headings;
	PackedVector3Array tip_headings;
	Vector<real_t> heading_weights;
	Vector3 direction_priorities;

protected:
	static void _bind_methods();

public:
	IKEffector3D() = default;
	void set_weight(real_t p_weight);
	real_t get_weight() const;
	void set_direction_priorities(Vector3 p_direction_priorities);
	Vector3 get_direction_priorities() const;
	void update_target_global_transform(Skeleton3D *p_skeleton, ManyBoneIK3D *p_modification = nullptr);
	const float MAX_KUSUDAMA_OPEN_CONES = 30;
	float get_passthrough_factor() const;
	void set_passthrough_factor(float p_passthrough_factor);
	void set_target_node(Skeleton3D *p_skeleton, const NodePath &p_target_node_path);
	NodePath get_target_node() const;
	Transform3D get_target_global_transform() const;
	void set_target_node_rotation(bool p_use);
	bool get_target_node_rotation() const;
	Ref<IKBone3D> get_ik_bone_3d() const;
	bool is_following_translation_only() const;
	int32_t update_effector_target_headings(PackedVector3Array *p_headings, int32_t p_index, Ref<IKBone3D> p_for_bone, const Vector<double> *p_weights) const;
	int32_t update_effector_tip_headings(PackedVector3Array *p_headings, int32_t p_index, Ref<IKBone3D> p_for_bone) const;
	IKEffector3D(const Ref<IKBone3D> &p_current_bone);
};


void IKEffector3D::set_target_node(Skeleton3D *p_skeleton, const NodePath &p_target_node_path) {
	ERR_FAIL_NULL(p_skeleton);
	target_node_path = p_target_node_path;
}

NodePath IKEffector3D::get_target_node() const {
	return target_node_path;
}

void IKEffector3D::set_target_node_rotation(bool p_use) {
	use_target_node_rotation = p_use;
}

bool IKEffector3D::get_target_node_rotation() const {
	return use_target_node_rotation;
}

Ref<IKBone3D> IKEffector3D::get_ik_bone_3d() const {
	return for_bone;
}

bool IKEffector3D::is_following_translation_only() const {
	return Math::is_zero_approx(direction_priorities.length_squared());
}

void IKEffector3D::set_direction_priorities(Vector3 p_direction_priorities) {
	direction_priorities = p_direction_priorities;
}

Vector3 IKEffector3D::get_direction_priorities() const {
	return direction_priorities;
}

void IKEffector3D::update_target_global_transform(Skeleton3D *p_skeleton, ManyBoneIK3D *p_many_bone_ik) {
	ERR_FAIL_NULL(p_skeleton);
	ERR_FAIL_NULL(for_bone);
	Node3D *current_target_node = cast_to<Node3D>(p_many_bone_ik->get_node_or_null(target_node_path));
	if (current_target_node && current_target_node->is_visible_in_tree()) {
		target_relative_to_skeleton_origin = p_skeleton->get_global_transform().affine_inverse() * current_target_node->get_global_transform();
	}
}

Transform3D IKEffector3D::get_target_global_transform() const {
	return target_relative_to_skeleton_origin;
}

int32_t IKEffector3D::update_effector_target_headings(PackedVector3Array *p_headings, int32_t p_index, Ref<IKBone3D> p_for_bone, const Vector<double> *p_weights) const {
	ERR_FAIL_COND_V(p_index == -1, -1);
	ERR_FAIL_NULL_V(p_headings, -1);
	ERR_FAIL_NULL_V(p_for_bone, -1);
	ERR_FAIL_NULL_V(p_weights, -1);

	int32_t index = p_index;
	Vector3 bone_origin_relative_to_skeleton_origin = for_bone->get_bone_direction_global_pose().origin;
	p_headings->write[index] = target_relative_to_skeleton_origin.origin - bone_origin_relative_to_skeleton_origin;
	index++;
	Vector3 priority = get_direction_priorities();
	for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
		if (priority[axis] > 0.0) {
			real_t w = p_weights->get(index);
			Vector3 column = target_relative_to_skeleton_origin.basis.get_column(axis);

			p_headings->write[index] = (column + target_relative_to_skeleton_origin.origin) - bone_origin_relative_to_skeleton_origin;
			p_headings->write[index] *= Vector3(w, w, w);
			index++;
			p_headings->write[index] = (target_relative_to_skeleton_origin.origin - column) - bone_origin_relative_to_skeleton_origin;
			p_headings->write[index] *= Vector3(w, w, w);
			index++;
		}
	}

	return index;
}

int32_t IKEffector3D::update_effector_tip_headings(PackedVector3Array *p_headings, int32_t p_index, Ref<IKBone3D> p_for_bone) const {
	ERR_FAIL_COND_V(p_index == -1, -1);
	ERR_FAIL_NULL_V(p_headings, -1);
	ERR_FAIL_NULL_V(p_for_bone, -1);

	Transform3D tip_xform_relative_to_skeleton_origin = for_bone->get_bone_direction_global_pose();
	Basis tip_basis = tip_xform_relative_to_skeleton_origin.basis;
	Vector3 bone_origin_relative_to_skeleton_origin = p_for_bone->get_bone_direction_global_pose().origin;

	int32_t index = p_index;
	p_headings->write[index] = tip_xform_relative_to_skeleton_origin.origin - bone_origin_relative_to_skeleton_origin;
	index++;
	double distance = target_relative_to_skeleton_origin.origin.distance_to(bone_origin_relative_to_skeleton_origin);
	double scale_by = MIN(distance, 1.0f);
	const Vector3 priority = get_direction_priorities();

	for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
		if (priority[axis] > 0.0) {
			Vector3 column = tip_basis.get_column(axis) * priority[axis];

			p_headings->write[index] = (column + tip_xform_relative_to_skeleton_origin.origin) - bone_origin_relative_to_skeleton_origin;
			p_headings->write[index] *= scale_by;
			index++;

			p_headings->write[index] = (tip_xform_relative_to_skeleton_origin.origin - column) - bone_origin_relative_to_skeleton_origin;
			p_headings->write[index] *= scale_by;
			index++;
		}
	}

	return index;
}

void IKEffector3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "skeleton", "node"),
			&IKEffector3D::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&IKEffector3D::get_target_node);
	ClassDB::bind_method(D_METHOD("set_passthrough_factor", "amount"),
			&IKEffector3D::set_passthrough_factor);
	ClassDB::bind_method(D_METHOD("get_passthrough_factor"),
			&IKEffector3D::get_passthrough_factor);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "passthrough_factor"), "set_passthrough_factor", "get_passthrough_factor");
}

void IKEffector3D::set_weight(real_t p_weight) {
	weight = p_weight;
}

real_t IKEffector3D::get_weight() const {
	return weight;
}

IKEffector3D::IKEffector3D(const Ref<IKBone3D> &p_current_bone) {
	ERR_FAIL_NULL(p_current_bone);
	for_bone = p_current_bone;
}

void IKEffector3D::set_passthrough_factor(float p_passthrough_factor) {
	passthrough_factor = CLAMP(p_passthrough_factor, 0.0, 1.0);
}

float IKEffector3D::get_passthrough_factor() const {
	return passthrough_factor;
}

class IKBoneSegment3D : public Resource {
	GDCLASS(IKBoneSegment3D, Resource);
	Ref<IKBone3D> root;
	Ref<IKBone3D> tip;
	Vector<Ref<IKBone3D>> bones;
	Vector<Ref<IKBone3D>> pinned_bones;
	Vector<Ref<IKBoneSegment3D>> child_segments; // Contains only direct child chains that end with effectors or have child that end with effectors
	Ref<IKBoneSegment3D> parent_segment;
	Ref<IKBoneSegment3D> root_segment;
	Vector<Ref<IKEffector3D>> effector_list;
	PackedVector3Array target_headings;
	PackedVector3Array tip_headings;
	PackedVector3Array tip_headings_uniform;
	Vector<double> heading_weights;
	Skeleton3D *skeleton = nullptr;
	bool pinned_descendants = false;
	double previous_deviation = INFINITY;
	int32_t default_stabilizing_pass_count = 0; // Move to the stabilizing pass to the ik solver. Set it free.
	bool _has_pinned_descendants();
	void _enable_pinned_descendants();
	void _update_target_headings(Ref<IKBone3D> p_for_bone, Vector<double> *r_weights, PackedVector3Array *r_htarget);
	void _update_tip_headings(Ref<IKBone3D> p_for_bone, PackedVector3Array *r_heading_tip);
	void _set_optimal_rotation(Ref<IKBone3D> p_for_bone, PackedVector3Array *r_htip, PackedVector3Array *r_heading_tip, Vector<double> *r_weights, float p_dampening = -1, bool p_translate = false, bool p_constraint_mode = false, int32_t current_iteration = 0, int32_t total_iterations = 0);
	void _qcp_solver(const Vector<float> &p_damp, float p_default_damp, bool p_translate, bool p_constraint_mode, int32_t p_current_iteration, int32_t p_total_iterations);
	void _update_optimal_rotation(Ref<IKBone3D> p_for_bone, double p_damp, bool p_translate, bool p_constraint_mode, int32_t current_iteration, int32_t total_iterations);
	float _get_manual_msd(const PackedVector3Array &r_htip, const PackedVector3Array &r_htarget, const Vector<double> &p_weights);
	HashMap<BoneId, Ref<IKBone3D>> bone_map;
	bool _is_parent_of_tip(Ref<IKBone3D> p_current_tip, BoneId p_tip_bone);
	bool _has_multiple_children_or_pinned(Vector<BoneId> &r_children, Ref<IKBone3D> p_current_tip);
	void _process_children(Vector<BoneId> &r_children, Ref<IKBone3D> p_current_tip, Vector<Ref<IKEffectorTemplate3D>> &r_pins, BoneId p_root_bone, BoneId p_tip_bone, ManyBoneIK3D *p_many_bone_ik);
	Ref<IKBoneSegment3D> _create_child_segment(String &p_child_name, Vector<Ref<IKEffectorTemplate3D>> &p_pins, BoneId p_root_bone, BoneId p_tip_bone, ManyBoneIK3D *p_many_bone_ik, Ref<IKBoneSegment3D> &p_parent);
	Ref<IKBone3D> _create_next_bone(BoneId p_bone_id, Ref<IKBone3D> p_current_tip, Vector<Ref<IKEffectorTemplate3D>> &p_pins, ManyBoneIK3D *p_many_bone_ik);
	void _finalize_segment(Ref<IKBone3D> p_current_tip);

protected:
	static void _bind_methods();

public:
	const double evec_prec = static_cast<double>(1E-6);
	void update_pinned_list(Vector<Vector<double>> &r_weights);
	static Quaternion clamp_to_cos_half_angle(Quaternion p_quat, double p_cos_half_angle);
	static void recursive_create_headings_arrays_for(Ref<IKBoneSegment3D> p_bone_segment);
	void create_headings_arrays();
	void recursive_create_penalty_array(Ref<IKBoneSegment3D> p_bone_segment, Vector<Vector<double>> &r_penalty_array, Vector<Ref<IKBone3D>> &r_pinned_bones, double p_falloff);
	void segment_solver(const Vector<float> &p_damp, float p_default_damp, bool p_constraint_mode, int32_t p_current_iteration, int32_t p_total_iteration);
	Ref<IKBone3D> get_root() const;
	Ref<IKBone3D> get_tip() const;
	bool is_pinned() const;
	Vector<Ref<IKBoneSegment3D>> get_child_segments() const;
	void create_bone_list(Vector<Ref<IKBone3D>> &p_list, bool p_recursive = false, bool p_debug_skeleton = false) const;
	Ref<IKBone3D> get_ik_bone(BoneId p_bone) const;
	void generate_default_segments(Vector<Ref<IKEffectorTemplate3D>> &p_pins, BoneId p_root_bone, BoneId p_tip_bone, ManyBoneIK3D *p_many_bone_ik);
	IKBoneSegment3D() {}
	IKBoneSegment3D(Skeleton3D *p_skeleton, StringName p_root_bone_name, Vector<Ref<IKEffectorTemplate3D>> &p_pins, ManyBoneIK3D *p_many_bone_ik, const Ref<IKBoneSegment3D> &p_parent = nullptr,
			BoneId root = -1, BoneId tip = -1, int32_t p_stabilizing_pass_count = 0);
	~IKBoneSegment3D() {}
};


Ref<IKBone3D> IKBoneSegment3D::get_root() const {
	return root;
}

Ref<IKBone3D> IKBoneSegment3D::get_tip() const {
	return tip;
}

bool IKBoneSegment3D::is_pinned() const {
	ERR_FAIL_NULL_V(tip, false);
	return tip->is_pinned();
}

Vector<Ref<IKBoneSegment3D>> IKBoneSegment3D::get_child_segments() const {
	return child_segments;
}

void IKBoneSegment3D::create_bone_list(Vector<Ref<IKBone3D>> &p_list, bool p_recursive, bool p_debug_skeleton) const {
	if (p_recursive) {
		for (int32_t child_i = 0; child_i < child_segments.size(); child_i++) {
			child_segments[child_i]->create_bone_list(p_list, p_recursive, p_debug_skeleton);
		}
	}
	Ref<IKBone3D> current_bone = tip;
	Vector<Ref<IKBone3D>> list;
	while (current_bone.is_valid()) {
		list.push_back(current_bone);
		if (current_bone == root) {
			break;
		}
		current_bone = current_bone->get_parent();
	}
	if (p_debug_skeleton) {
		for (int32_t name_i = 0; name_i < list.size(); name_i++) {
			BoneId bone = list[name_i]->get_bone_id();

			String bone_name = skeleton->get_bone_name(bone);
			String effector;
			if (list[name_i]->is_pinned()) {
				effector += "Effector ";
			}
			String prefix;
			if (list[name_i] == root) {
				prefix += "(" + effector + "Root) ";
			}
			if (list[name_i] == tip) {
				prefix += "(" + effector + "Tip) ";
			}
			print_line(vformat("%s%s (%s)", prefix, bone_name, itos(bone)));
		}
	}
	p_list.append_array(list);
}

void IKBoneSegment3D::update_pinned_list(Vector<Vector<double>> &r_weights) {
	for (int32_t chain_i = 0; chain_i < child_segments.size(); chain_i++) {
		Ref<IKBoneSegment3D> chain = child_segments[chain_i];
		chain->update_pinned_list(r_weights);
	}
	if (is_pinned()) {
		effector_list.push_back(tip->get_pin());
	}
	double passthrough_factor = is_pinned() ? tip->get_pin()->passthrough_factor : 1.0;
	if (passthrough_factor > 0.0) {
		for (Ref<IKBoneSegment3D> child : child_segments) {
			effector_list.append_array(child->effector_list);
		}
	}
}

void IKBoneSegment3D::_update_optimal_rotation(Ref<IKBone3D> p_for_bone, double p_damp, bool p_translate, bool p_constraint_mode, int32_t current_iteration, int32_t total_iterations) {
	ERR_FAIL_NULL(p_for_bone);
	_update_target_headings(p_for_bone, &heading_weights, &target_headings);
	_update_tip_headings(p_for_bone, &tip_headings);
	_set_optimal_rotation(p_for_bone, &tip_headings, &target_headings, &heading_weights, p_damp, p_translate, p_constraint_mode);
}

Quaternion IKBoneSegment3D::clamp_to_cos_half_angle(Quaternion p_quat, double p_cos_half_angle) {
	if (p_quat.w < 0.0) {
		p_quat = p_quat * -1;
	}
	double previous_coefficient = (1.0 - (p_quat.w * p_quat.w));
	if (p_cos_half_angle <= p_quat.w || previous_coefficient == 0.0) {
		return p_quat;
	} else {
		double composite_coefficient = Math::sqrt((1.0 - (p_cos_half_angle * p_cos_half_angle)) / previous_coefficient);
		p_quat.w = p_cos_half_angle;
		p_quat.x *= composite_coefficient;
		p_quat.y *= composite_coefficient;
		p_quat.z *= composite_coefficient;
	}
	return p_quat;
}

float IKBoneSegment3D::_get_manual_msd(const PackedVector3Array &r_htip, const PackedVector3Array &r_htarget, const Vector<double> &p_weights) {
	float manual_RMSD = 0.0f;
	float w_sum = 0.0f;
	for (int i = 0; i < r_htarget.size(); i++) {
		float x_d = r_htarget[i].x - r_htip[i].x;
		float y_d = r_htarget[i].y - r_htip[i].y;
		float z_d = r_htarget[i].z - r_htip[i].z;
		float mag_sq = p_weights[i] * (x_d * x_d + y_d * y_d + z_d * z_d);
		manual_RMSD += mag_sq;
		w_sum += p_weights[i];
	}
	manual_RMSD /= w_sum * w_sum;
	return manual_RMSD;
}

void IKBoneSegment3D::_set_optimal_rotation(Ref<IKBone3D> p_for_bone, PackedVector3Array *r_htip, PackedVector3Array *r_htarget, Vector<double> *r_weights, float p_dampening, bool p_translate, bool p_constraint_mode, int32_t current_iteration, int32_t total_iterations) {
	ERR_FAIL_NULL(p_for_bone);
	ERR_FAIL_NULL(r_htip);
	ERR_FAIL_NULL(r_htarget);
	ERR_FAIL_NULL(r_weights);

	_update_target_headings(p_for_bone, &heading_weights, &target_headings);
	Transform3D prev_transform = p_for_bone->get_pose();
	bool got_closer = true;
	double bone_damp = p_for_bone->get_cos_half_dampen();
	int i = 0;
	do {
		_update_tip_headings(p_for_bone, &tip_headings);
		if (!p_constraint_mode) {
			QCP qcp = QCP(evec_prec);
			Basis rotation = qcp.weighted_superpose(*r_htip, *r_htarget, *r_weights, p_translate);
			Vector3 translation = qcp.get_translation();
			double dampening = (p_dampening != -1.0) ? p_dampening : bone_damp;
			rotation = clamp_to_cos_half_angle(rotation.get_rotation_quaternion(), cos(dampening / 2.0));
			p_for_bone->get_ik_transform()->rotate_local_with_global(rotation.get_rotation_quaternion());
			Transform3D result = Transform3D(p_for_bone->get_global_pose().basis, p_for_bone->get_global_pose().origin + translation);
			p_for_bone->set_global_pose(result);
		}
		bool is_parent_valid = p_for_bone->get_parent().is_valid();
		if (is_parent_valid && p_for_bone->is_orientationally_constrained()) {
			p_for_bone->get_constraint()->snap_to_orientation_limit(p_for_bone->get_bone_direction_transform(), p_for_bone->get_ik_transform(), p_for_bone->get_constraint_orientation_transform(), bone_damp, p_for_bone->get_cos_half_dampen());
		}
		if (is_parent_valid && p_for_bone->is_axially_constrained()) {
			p_for_bone->get_constraint()->set_snap_to_twist_limit(p_for_bone->get_bone_direction_transform(), p_for_bone->get_ik_transform(), p_for_bone->get_constraint_twist_transform(), bone_damp, p_for_bone->get_cos_half_dampen());
		}
		if (default_stabilizing_pass_count > 0) {
			_update_tip_headings(p_for_bone, &tip_headings_uniform);
			double current_msd = _get_manual_msd(tip_headings_uniform, target_headings, heading_weights);
			if (current_msd <= previous_deviation * 1.0001) {
				previous_deviation = current_msd;
				got_closer = true;
				break;
			} else {
				got_closer = false;
				p_for_bone->set_pose(prev_transform);
			}
		}
		i++;
	} while (i < default_stabilizing_pass_count && !got_closer);

	if (root == p_for_bone) {
		previous_deviation = INFINITY;
	}
}

void IKBoneSegment3D::_update_target_headings(Ref<IKBone3D> p_for_bone, Vector<double> *r_weights, PackedVector3Array *r_target_headings) {
	ERR_FAIL_NULL(p_for_bone);
	ERR_FAIL_NULL(r_weights);
	ERR_FAIL_NULL(r_target_headings);
	int32_t last_index = 0;
	for (int32_t effector_i = 0; effector_i < effector_list.size(); effector_i++) {
		Ref<IKEffector3D> effector = effector_list[effector_i];
		if (effector.is_null()) {
			continue;
		}
		last_index = effector->update_effector_target_headings(r_target_headings, last_index, p_for_bone, &heading_weights);
	}
}

void IKBoneSegment3D::_update_tip_headings(Ref<IKBone3D> p_for_bone, PackedVector3Array *r_heading_tip) {
	ERR_FAIL_NULL(r_heading_tip);
	ERR_FAIL_NULL(p_for_bone);
	int32_t last_index = 0;
	for (int32_t effector_i = 0; effector_i < effector_list.size(); effector_i++) {
		Ref<IKEffector3D> effector = effector_list[effector_i];
		if (effector.is_null()) {
			continue;
		}
		last_index = effector->update_effector_tip_headings(r_heading_tip, last_index, p_for_bone);
	}
}

void IKBoneSegment3D::segment_solver(const Vector<float> &p_damp, float p_default_damp, bool p_constraint_mode, int32_t p_current_iteration, int32_t p_total_iteration) {
	for (Ref<IKBoneSegment3D> child : child_segments) {
		if (child.is_null()) {
			continue;
		}
		child->segment_solver(p_damp, p_default_damp, p_constraint_mode, p_current_iteration, p_total_iteration);
	}
	bool is_translate = parent_segment.is_null();
	if (is_translate) {
		Vector<float> damp = p_damp;
		damp.fill(Math_PI);
		_qcp_solver(damp, Math_PI, is_translate, p_constraint_mode, p_current_iteration, p_total_iteration);
		return;
	}
	_qcp_solver(p_damp, p_default_damp, is_translate, p_constraint_mode, p_current_iteration, p_total_iteration);
}

void IKBoneSegment3D::_qcp_solver(const Vector<float> &p_damp, float p_default_damp, bool p_translate, bool p_constraint_mode, int32_t p_current_iteration, int32_t p_total_iterations) {
	for (Ref<IKBone3D> current_bone : bones) {
		float damp = p_default_damp;
		bool is_valid_access = !(unlikely((p_damp.size()) < 0 || (current_bone->get_bone_id()) >= (p_damp.size())));
		if (is_valid_access) {
			damp = p_damp[current_bone->get_bone_id()];
		}
		bool is_non_default_damp = p_default_damp < damp;
		if (is_non_default_damp) {
			damp = p_default_damp;
		}
		_update_optimal_rotation(current_bone, damp, p_translate, p_constraint_mode, p_current_iteration, p_total_iterations);
	}
}

void IKBoneSegment3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_pinned"), &IKBoneSegment3D::is_pinned);
	ClassDB::bind_method(D_METHOD("get_ik_bone", "bone"), &IKBoneSegment3D::get_ik_bone);
}

IKBoneSegment3D::IKBoneSegment3D(Skeleton3D *p_skeleton, StringName p_root_bone_name, Vector<Ref<IKEffectorTemplate3D>> &p_pins, ManyBoneIK3D *p_many_bone_ik, const Ref<IKBoneSegment3D> &p_parent,
		BoneId p_root, BoneId p_tip, int32_t p_stabilizing_pass_count) {
	root = p_root;
	tip = p_tip;
	skeleton = p_skeleton;
	root = Ref<IKBone3D>(memnew(IKBone3D(p_root_bone_name, p_skeleton, p_parent, p_pins, Math_PI, p_many_bone_ik)));
	if (p_parent.is_valid()) {
		root_segment = p_parent->root_segment;
	} else {
		root_segment = Ref<IKBoneSegment3D>(this);
	}
	root_segment->bone_map[root->get_bone_id()] = root;
	if (p_parent.is_valid()) {
		parent_segment = p_parent;
		root->set_parent(p_parent->get_tip());
	}
	default_stabilizing_pass_count = p_stabilizing_pass_count;
}

void IKBoneSegment3D::_enable_pinned_descendants() {
	pinned_descendants = true;
}

bool IKBoneSegment3D::_has_pinned_descendants() {
	return pinned_descendants;
}

Ref<IKBone3D> IKBoneSegment3D::get_ik_bone(BoneId p_bone) const {
	if (!bone_map.has(p_bone)) {
		return Ref<IKBone3D>();
	}
	return bone_map[p_bone];
}

void IKBoneSegment3D::create_headings_arrays() {
	Vector<Vector<double>> penalty_array;
	Vector<Ref<IKBone3D>> new_pinned_bones;
	recursive_create_penalty_array(this, penalty_array, new_pinned_bones, 1.0);
	pinned_bones.resize(new_pinned_bones.size());
	int32_t total_headings = 0;
	for (const Vector<double> &current_penalty_array : penalty_array) {
		total_headings += current_penalty_array.size();
	}
	for (int32_t bone_i = 0; bone_i < new_pinned_bones.size(); bone_i++) {
		pinned_bones.write[bone_i] = new_pinned_bones[bone_i];
	}
	target_headings.resize(total_headings);
	tip_headings.resize(total_headings);
	tip_headings_uniform.resize(total_headings);
	heading_weights.resize(total_headings);
	int currentHeading = 0;
	for (const Vector<double> &current_penalty_array : penalty_array) {
		for (double ad : current_penalty_array) {
			heading_weights.write[currentHeading] = ad;
			target_headings.write[currentHeading] = Vector3();
			tip_headings.write[currentHeading] = Vector3();
			tip_headings_uniform.write[currentHeading] = Vector3();
			currentHeading++;
		}
	}
}

void IKBoneSegment3D::recursive_create_penalty_array(Ref<IKBoneSegment3D> p_bone_segment, Vector<Vector<double>> &r_penalty_array, Vector<Ref<IKBone3D>> &r_pinned_bones, double p_falloff) {
	if (p_falloff <= 0.0) {
		return;
	}

	double current_falloff = 1.0;

	if (p_bone_segment->is_pinned()) {
		Ref<IKBone3D> current_tip = p_bone_segment->get_tip();
		Ref<IKEffector3D> pin = current_tip->get_pin();
		double weight = pin->get_weight();
		Vector<double> inner_weight_array;
		inner_weight_array.push_back(weight * p_falloff);

		double max_pin_weight = MAX(MAX(pin->get_direction_priorities().x, pin->get_direction_priorities().y), pin->get_direction_priorities().z);
		max_pin_weight = max_pin_weight == 0.0 ? 1.0 : max_pin_weight;

		for (int i = 0; i < 3; ++i) {
			double priority = pin->get_direction_priorities()[i];
			if (priority > 0.0) {
				double sub_target_weight = weight * (priority / max_pin_weight) * p_falloff;
				inner_weight_array.push_back(sub_target_weight);
				inner_weight_array.push_back(sub_target_weight);
			}
		}

		r_penalty_array.push_back(inner_weight_array);
		r_pinned_bones.push_back(current_tip);
		current_falloff = pin->get_passthrough_factor();
	}

	for (Ref<IKBoneSegment3D> s : p_bone_segment->get_child_segments()) {
		recursive_create_penalty_array(s, r_penalty_array, r_pinned_bones, p_falloff * current_falloff);
	}
}

void IKBoneSegment3D::recursive_create_headings_arrays_for(Ref<IKBoneSegment3D> p_bone_segment) {
	p_bone_segment->create_headings_arrays();
	for (Ref<IKBoneSegment3D> segments : p_bone_segment->get_child_segments()) {
		recursive_create_headings_arrays_for(segments);
	}
}

void IKBoneSegment3D::generate_default_segments(Vector<Ref<IKEffectorTemplate3D>> &p_pins, BoneId p_root_bone, BoneId p_tip_bone, ManyBoneIK3D *p_many_bone_ik) {
	Ref<IKBone3D> current_tip = root;
	Vector<BoneId> children;

	while (!_is_parent_of_tip(current_tip, p_tip_bone)) {
		children = skeleton->get_bone_children(current_tip->get_bone_id());

		if (children.is_empty() || _has_multiple_children_or_pinned(children, current_tip)) {
			_process_children(children, current_tip, p_pins, p_root_bone, p_tip_bone, p_many_bone_ik);
			break;
		} else {
			Vector<BoneId>::Iterator bone_id_iterator = children.begin();
			current_tip = _create_next_bone(*bone_id_iterator, current_tip, p_pins, p_many_bone_ik);
		}
	}

	_finalize_segment(current_tip);
}

bool IKBoneSegment3D::_is_parent_of_tip(Ref<IKBone3D> p_current_tip, BoneId p_tip_bone) {
	return skeleton->get_bone_parent(p_current_tip->get_bone_id()) >= p_tip_bone && p_tip_bone != -1;
}

bool IKBoneSegment3D::_has_multiple_children_or_pinned(Vector<BoneId> &r_children, Ref<IKBone3D> p_current_tip) {
	return r_children.size() > 1 || p_current_tip->is_pinned();
}

void IKBoneSegment3D::_process_children(Vector<BoneId> &r_children, Ref<IKBone3D> p_current_tip, Vector<Ref<IKEffectorTemplate3D>> &r_pins, BoneId p_root_bone, BoneId p_tip_bone, ManyBoneIK3D *p_many_bone_ik) {
	tip = p_current_tip;
	Ref<IKBoneSegment3D> parent(this);

	for (int32_t child_i = 0; child_i < r_children.size(); child_i++) {
		BoneId child_bone = r_children[child_i];
		String child_name = skeleton->get_bone_name(child_bone);
		Ref<IKBoneSegment3D> child_segment = _create_child_segment(child_name, r_pins, p_root_bone, p_tip_bone, p_many_bone_ik, parent);

		child_segment->generate_default_segments(r_pins, p_root_bone, p_tip_bone, p_many_bone_ik);

		if (child_segment->_has_pinned_descendants()) {
			_enable_pinned_descendants();
			child_segments.push_back(child_segment);
		}
	}
}

Ref<IKBoneSegment3D> IKBoneSegment3D::_create_child_segment(String &p_child_name, Vector<Ref<IKEffectorTemplate3D>> &p_pins, BoneId p_root_bone, BoneId p_tip_bone, ManyBoneIK3D *p_many_bone_ik, Ref<IKBoneSegment3D> &p_parent) {
	return Ref<IKBoneSegment3D>(memnew(IKBoneSegment3D(skeleton, p_child_name, p_pins, p_many_bone_ik, p_parent, p_root_bone, p_tip_bone)));
}

Ref<IKBone3D> IKBoneSegment3D::_create_next_bone(BoneId p_bone_id, Ref<IKBone3D> p_current_tip, Vector<Ref<IKEffectorTemplate3D>> &p_pins, ManyBoneIK3D *p_many_bone_ik) {
	String bone_name = skeleton->get_bone_name(p_bone_id);
	Ref<IKBone3D> next_bone = Ref<IKBone3D>(memnew(IKBone3D(bone_name, skeleton, p_current_tip, p_pins, p_many_bone_ik->get_default_damp(), p_many_bone_ik)));
	root_segment->bone_map[p_bone_id] = next_bone;

	return next_bone;
}

void IKBoneSegment3D::_finalize_segment(Ref<IKBone3D> p_current_tip) {
	tip = p_current_tip;

	if (tip->is_pinned()) {
		_enable_pinned_descendants();
	}

	StringBuilder name_builder;
	name_builder.append("IKBoneSegment");
	name_builder.append(root->get_name());
	name_builder.append("Root");
	name_builder.append(tip->get_name());
	name_builder.append("Tip");

	String ik_bone_name = name_builder.as_string();
	set_name(ik_bone_name);
	bones.clear();
	create_bone_list(bones, false);
}

class IKBone3D : public Resource {
	GDCLASS(IKBone3D, Resource);

	BoneId bone_id = -1;
	Ref<IKBone3D> parent;
	Vector<Ref<IKBone3D>> children;
	Ref<IKEffector3D> pin;

	float default_dampening = Math_PI;
	float dampening = get_parent().is_null() ? Math_PI : default_dampening;
	float cos_half_dampen = Math::cos(dampening / 2.0f);
	double cos_half_return_damp = 0.0f;
	double return_damp = 0.0f;
	Vector<float> cos_half_returnfulness_dampened;
	Vector<float> half_returnfulness_dampened;
	double stiffness = 0.0;
	Ref<IKKusudama3D> constraint;
	// In the space of the local parent bone transform.
	// The origin is the origin of the bone direction transform
	// Can be independent and should be calculated
	// to keep -y to be the opposite of its bone forward orientation
	// To avoid singularity that is ambiguous.
	Ref<IKNode3D> constraint_orientation_transform = Ref<IKNode3D>(memnew(IKNode3D()));
	Ref<IKNode3D> constraint_twist_transform = Ref<IKNode3D>(memnew(IKNode3D()));
	Ref<IKNode3D> godot_skeleton_aligned_transform = Ref<IKNode3D>(memnew(IKNode3D())); // The bone's actual transform.
	Ref<IKNode3D> bone_direction_transform = Ref<IKNode3D>(memnew(IKNode3D())); // Physical direction of the bone. Calculate Y is the bone up.

protected:
	static void _bind_methods();

public:
	Vector<float> &get_cos_half_returnfullness_dampened();
	void set_cos_half_returnfullness_dampened(const Vector<float> &p_value);
	Vector<float> &get_half_returnfullness_dampened();
	void set_half_returnfullness_dampened(const Vector<float> &p_value);
	void set_stiffness(double p_stiffness);
	double get_stiffness() const;
	bool is_axially_constrained();
	bool is_orientationally_constrained();
	Transform3D get_bone_direction_global_pose() const;
	Ref<IKNode3D> get_bone_direction_transform();
	void set_bone_direction_transform(Ref<IKNode3D> p_bone_direction);
	void update_default_bone_direction_transform(Skeleton3D *p_skeleton);
	void set_constraint_orientation_transform(Ref<IKNode3D> p_transform);
	Ref<IKNode3D> get_constraint_orientation_transform();
	Ref<IKNode3D> get_constraint_twist_transform();
	void update_default_constraint_transform();
	void add_constraint(Ref<IKKusudama3D> p_constraint);
	Ref<IKKusudama3D> get_constraint() const;
	void set_bone_id(BoneId p_bone_id, Skeleton3D *p_skeleton = nullptr);
	BoneId get_bone_id() const;
	void set_parent(const Ref<IKBone3D> &p_parent);
	Ref<IKBone3D> get_parent() const;
	void set_pin(const Ref<IKEffector3D> &p_pin);
	Ref<IKEffector3D> get_pin() const;
	void set_global_pose(const Transform3D &p_transform);
	Transform3D get_global_pose() const;
	void set_pose(const Transform3D &p_transform);
	Transform3D get_pose() const;
	void set_initial_pose(Skeleton3D *p_skeleton);
	void set_skeleton_bone_pose(Skeleton3D *p_skeleton);
	void create_pin();
	bool is_pinned() const;
	Ref<IKNode3D> get_ik_transform();
	IKBone3D() {}
	IKBone3D(StringName p_bone, Skeleton3D *p_skeleton, const Ref<IKBone3D> &p_parent, Vector<Ref<IKEffectorTemplate3D>> &p_pins, float p_default_dampening = Math_PI, ManyBoneIK3D *p_many_bone_ik = nullptr);
	~IKBone3D() {}
	float get_cos_half_dampen() const;
	void set_cos_half_dampen(float p_cos_half_dampen);
	Transform3D get_parent_bone_aligned_transform();
	Transform3D get_set_constraint_twist_transform() const;
	float calculate_total_radius_sum(const TypedArray<IKOpenCone3D> &p_cones) const;
	Vector3 calculate_weighted_direction(const TypedArray<IKOpenCone3D> &p_cones, float p_total_radius_sum) const;
};


void IKBone3D::set_bone_id(BoneId p_bone_id, Skeleton3D *p_skeleton) {
	ERR_FAIL_NULL(p_skeleton);
	bone_id = p_bone_id;
}

BoneId IKBone3D::get_bone_id() const {
	return bone_id;
}

void IKBone3D::set_parent(const Ref<IKBone3D> &p_parent) {
	ERR_FAIL_NULL(p_parent);
	parent = p_parent;
	if (parent.is_valid()) {
		parent->children.push_back(this);
		godot_skeleton_aligned_transform->set_parent(parent->godot_skeleton_aligned_transform);
		constraint_orientation_transform->set_parent(parent->godot_skeleton_aligned_transform);
		constraint_twist_transform->set_parent(parent->godot_skeleton_aligned_transform);
	}
}

void IKBone3D::update_default_bone_direction_transform(Skeleton3D *p_skeleton) {
	Vector3 child_centroid;
	int child_count = 0;

	for (Ref<IKBone3D> &ik_bone : children) {
		child_centroid += ik_bone->get_ik_transform()->get_global_transform().origin;
		child_count++;
	}

	if (child_count > 0) {
		child_centroid /= child_count;
	} else {
		const PackedInt32Array &bone_children = p_skeleton->get_bone_children(bone_id);
		for (BoneId child_bone_idx : bone_children) {
			child_centroid += p_skeleton->get_bone_global_pose(child_bone_idx).origin;
		}
		child_centroid /= bone_children.size();
	}

	const Vector3 &godot_bone_origin = godot_skeleton_aligned_transform->get_global_transform().origin;
	child_centroid -= godot_bone_origin;

	if (Math::is_zero_approx(child_centroid.length_squared())) {
		if (parent.is_valid()) {
			child_centroid = parent->get_bone_direction_transform()->get_global_transform().basis.get_column(Vector3::AXIS_Y);
		} else {
			child_centroid = get_bone_direction_transform()->get_global_transform().basis.get_column(Vector3::AXIS_Y);
		}
	}

	if (!Math::is_zero_approx(child_centroid.length_squared()) && (children.size() || p_skeleton->get_bone_children(bone_id).size())) {
		child_centroid.normalize();
		Vector3 bone_direction = bone_direction_transform->get_global_transform().basis.get_column(Vector3::AXIS_Y);
		bone_direction.normalize();
		bone_direction_transform->rotate_local_with_global(Quaternion(child_centroid, bone_direction));
	}
}

void IKBone3D::update_default_constraint_transform() {
	Ref<IKBone3D> parent_bone = get_parent();
	if (parent_bone.is_valid()) {
		Transform3D parent_bone_aligned_transform = get_parent_bone_aligned_transform();
		constraint_orientation_transform->set_global_transform(parent_bone_aligned_transform);
	}

	Transform3D set_constraint_twist_transform = get_set_constraint_twist_transform();
	constraint_twist_transform->set_global_transform(set_constraint_twist_transform);

	if (constraint.is_null()) {
		return;
	}

	TypedArray<IKOpenCone3D> cones = constraint->get_open_cones();
	Vector3 direction;
	if (cones.size() == 0) {
		direction = bone_direction_transform->get_global_transform().basis.get_column(Vector3::AXIS_Y);
	} else {
		float total_radius_sum = calculate_total_radius_sum(cones);
		direction = calculate_weighted_direction(cones, total_radius_sum);
		direction -= constraint_orientation_transform->get_global_transform().origin;
	}

	Vector3 twist_axis = set_constraint_twist_transform.basis.get_column(Vector3::AXIS_Y);
	Quaternion align_dir = Quaternion(twist_axis, direction);
	constraint_twist_transform->rotate_local_with_global(align_dir);
}

Ref<IKBone3D> IKBone3D::get_parent() const {
	return parent;
}

void IKBone3D::set_pin(const Ref<IKEffector3D> &p_pin) {
	ERR_FAIL_NULL(p_pin);
	pin = p_pin;
}

Ref<IKEffector3D> IKBone3D::get_pin() const {
	return pin;
}

void IKBone3D::set_pose(const Transform3D &p_transform) {
	godot_skeleton_aligned_transform->set_transform(p_transform);
}

Transform3D IKBone3D::get_pose() const {
	return godot_skeleton_aligned_transform->get_transform();
}

void IKBone3D::set_global_pose(const Transform3D &p_transform) {
	godot_skeleton_aligned_transform->set_global_transform(p_transform);
	Transform3D transform = constraint_orientation_transform->get_transform();
	transform.origin = godot_skeleton_aligned_transform->get_transform().origin;
	constraint_orientation_transform->set_transform(transform);
	constraint_orientation_transform->_propagate_transform_changed();
}

Transform3D IKBone3D::get_global_pose() const {
	return godot_skeleton_aligned_transform->get_global_transform();
}

Transform3D IKBone3D::get_bone_direction_global_pose() const {
	return bone_direction_transform->get_global_transform();
}

void IKBone3D::set_initial_pose(Skeleton3D *p_skeleton) {
	ERR_FAIL_NULL(p_skeleton);
	if (bone_id == -1) {
		return;
	}
	Transform3D bone_origin_to_parent_origin = p_skeleton->get_bone_pose(bone_id);
	set_pose(bone_origin_to_parent_origin);
}

void IKBone3D::set_skeleton_bone_pose(Skeleton3D *p_skeleton) {
	ERR_FAIL_NULL(p_skeleton);
	Transform3D bone_to_parent = get_pose();
	p_skeleton->set_bone_pose_position(bone_id, bone_to_parent.origin);
	if (!bone_to_parent.basis.is_finite()) {
		bone_to_parent.basis = Basis();
	}
	p_skeleton->set_bone_pose_rotation(bone_id, bone_to_parent.basis.get_rotation_quaternion());
	p_skeleton->set_bone_pose_scale(bone_id, bone_to_parent.basis.get_scale());
}

void IKBone3D::create_pin() {
	pin = Ref<IKEffector3D>(memnew(IKEffector3D(this)));
}

bool IKBone3D::is_pinned() const {
	return pin.is_valid();
}

void IKBone3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_pin"), &IKBone3D::get_pin);
	ClassDB::bind_method(D_METHOD("set_pin", "pin"), &IKBone3D::set_pin);
	ClassDB::bind_method(D_METHOD("is_pinned"), &IKBone3D::is_pinned);
	ClassDB::bind_method(D_METHOD("get_constraint"), &IKBone3D::get_constraint);
	ClassDB::bind_method(D_METHOD("get_constraint_orientation_transform"), &IKBone3D::get_constraint_orientation_transform);
	ClassDB::bind_method(D_METHOD("get_constraint_twist_transform"), &IKBone3D::get_constraint_twist_transform);
}

IKBone3D::IKBone3D(StringName p_bone, Skeleton3D *p_skeleton, const Ref<IKBone3D> &p_parent, Vector<Ref<IKEffectorTemplate3D>> &p_pins, float p_default_dampening,
		ManyBoneIK3D *p_many_bone_ik) {
	ERR_FAIL_NULL(p_skeleton);

	default_dampening = p_default_dampening;
	cos_half_dampen = cos(default_dampening / real_t(2.0));
	set_name(p_bone);
	bone_id = p_skeleton->find_bone(p_bone);
	if (p_parent.is_valid()) {
		set_parent(p_parent);
	}
	for (Ref<IKEffectorTemplate3D> elem : p_pins) {
		if (elem.is_null()) {
			continue;
		}
		if (elem->get_name() == p_bone) {
			create_pin();
			Ref<IKEffector3D> effector = get_pin();
			effector->set_target_node(p_skeleton, elem->get_target_node());
			effector->set_passthrough_factor(elem->get_passthrough_factor());
			effector->set_weight(elem->get_weight());
			effector->set_direction_priorities(elem->get_direction_priorities());
			break;
		}
	}
	bone_direction_transform->set_parent(godot_skeleton_aligned_transform);

	float predamp = 1.0 - get_stiffness();
	dampening = get_parent().is_null() ? Math_PI : predamp * p_default_dampening;
	float iterations = p_many_bone_ik->get_iterations_per_frame();
	if (get_constraint().is_null()) {
		Ref<IKKusudama3D> new_constraint;
		new_constraint.instantiate();
		add_constraint(new_constraint);
	}
	float returnfulness = get_constraint()->get_resistance();
	float falloff = 0.2f;
	half_returnfulness_dampened.resize(iterations);
	cos_half_returnfulness_dampened.resize(iterations);
	float iterations_pow = Math::pow(iterations, falloff * iterations * returnfulness);
	for (float i = 0; i < iterations; i++) {
		float iteration_scalar = ((iterations_pow)-Math::pow(i, falloff * iterations * returnfulness)) / (iterations_pow);
		float iteration_return_clamp = iteration_scalar * returnfulness * dampening;
		float cos_iteration_return_clamp = Math::cos(iteration_return_clamp / 2.0);
		half_returnfulness_dampened.write[i] = iteration_return_clamp;
		cos_half_returnfulness_dampened.write[i] = cos_iteration_return_clamp;
	}
}

float IKBone3D::get_cos_half_dampen() const {
	return cos_half_dampen;
}

void IKBone3D::set_cos_half_dampen(float p_cos_half_dampen) {
	cos_half_dampen = p_cos_half_dampen;
}

Ref<IKKusudama3D> IKBone3D::get_constraint() const {
	return constraint;
}

void IKBone3D::add_constraint(Ref<IKKusudama3D> p_constraint) {
	constraint = p_constraint;
}

Ref<IKNode3D> IKBone3D::get_ik_transform() {
	return godot_skeleton_aligned_transform;
}

Ref<IKNode3D> IKBone3D::get_constraint_orientation_transform() {
	return constraint_orientation_transform;
}

Ref<IKNode3D> IKBone3D::get_constraint_twist_transform() {
	return constraint_twist_transform;
}

void IKBone3D::set_constraint_orientation_transform(Ref<IKNode3D> p_transform) {
	constraint_orientation_transform = p_transform;
}

void IKBone3D::set_bone_direction_transform(Ref<IKNode3D> p_bone_direction) {
	bone_direction_transform = p_bone_direction;
}

Ref<IKNode3D> IKBone3D::get_bone_direction_transform() {
	return bone_direction_transform;
}

bool IKBone3D::is_orientationally_constrained() {
	if (get_constraint().is_null()) {
		return false;
	}
	return get_constraint()->is_orientationally_constrained();
}

bool IKBone3D::is_axially_constrained() {
	if (get_constraint().is_null()) {
		return false;
	}
	return get_constraint()->is_axially_constrained();
}

Vector<float> &IKBone3D::get_cos_half_returnfullness_dampened() {
	return cos_half_returnfulness_dampened;
}

void IKBone3D::set_cos_half_returnfullness_dampened(const Vector<float> &p_value) {
	cos_half_returnfulness_dampened = p_value;
}

Vector<float> &IKBone3D::get_half_returnfullness_dampened() {
	return half_returnfulness_dampened;
}

void IKBone3D::set_half_returnfullness_dampened(const Vector<float> &p_value) {
	half_returnfulness_dampened = p_value;
}

void IKBone3D::set_stiffness(double p_stiffness) {
	stiffness = p_stiffness;
}

double IKBone3D::get_stiffness() const {
	return stiffness;
}

Transform3D IKBone3D::get_parent_bone_aligned_transform() {
	Ref<IKBone3D> parent_bone = get_parent();
	if (parent_bone.is_null()) {
		return Transform3D();
	}
	Transform3D parent_bone_aligned_transform = parent_bone->get_ik_transform()->get_global_transform();
	parent_bone_aligned_transform.origin = get_bone_direction_transform()->get_global_transform().origin;
	return parent_bone_aligned_transform;
}

Transform3D IKBone3D::get_set_constraint_twist_transform() const {
	return constraint_orientation_transform->get_global_transform();
}

float IKBone3D::calculate_total_radius_sum(const TypedArray<IKOpenCone3D> &p_cones) const {
	float total_radius_sum = 0.0f;
	for (int32_t i = 0; i < p_cones.size(); ++i) {
		const Ref<IKOpenCone3D> &cone = p_cones[i];
		if (cone.is_null()) {
			break;
		}
		total_radius_sum += cone->get_radius();
	}
	return total_radius_sum;
}

Vector3 IKBone3D::calculate_weighted_direction(const TypedArray<IKOpenCone3D> &p_cones, float p_total_radius_sum) const {
	Vector3 direction = Vector3();
	for (int32_t i = 0; i < p_cones.size(); ++i) {
		const Ref<IKOpenCone3D> &cone = p_cones[i];
		if (cone.is_null()) {
			break;
		}
		float weight = cone->get_radius() / p_total_radius_sum;
		direction += cone->get_control_point() * weight;
	}
	direction.normalize();
	direction = constraint_orientation_transform->get_global_transform().basis.xform(direction);
	return direction;
}
