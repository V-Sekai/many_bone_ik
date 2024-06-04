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
