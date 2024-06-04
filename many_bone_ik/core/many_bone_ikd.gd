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
