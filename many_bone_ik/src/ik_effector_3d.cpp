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
