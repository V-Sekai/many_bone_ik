#include "bone_effector.h"

void BoneEffector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_transform", "transform"),
			&BoneEffector::set_target_transform);
	ClassDB::bind_method(D_METHOD("get_target_transform"),
			&BoneEffector::get_target_transform);

	ClassDB::bind_method(D_METHOD("set_target_node", "node"),
			&BoneEffector::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&BoneEffector::get_target_node);

	ClassDB::bind_method(D_METHOD("set_budget", "ms"),
			&BoneEffector::set_budget_ms);
	ClassDB::bind_method(D_METHOD("get_budget"),
			&BoneEffector::get_budget_ms);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "target_transform"), "set_target_transform", "get_target_transform");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "budget"), "set_budget", "get_budget");
}

void BoneEffector::set_target_transform(Transform p_target_transform) {
	target_transform = p_target_transform;
}

Transform BoneEffector::get_target_transform() const {
	return target_transform;
}

void BoneEffector::set_target_node(NodePath p_target_node_path) {
	target_node = p_target_node_path;
}

NodePath BoneEffector::get_target_node() const {
	return target_node;
}

real_t BoneEffector::get_budget_ms() const {
	return budget_ms;
}

void BoneEffector::set_budget_ms(real_t p_budget_ms) {
	budget_ms = p_budget_ms;
}
