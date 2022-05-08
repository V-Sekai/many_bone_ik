#include "skeleton_modification_3d_ewbik.h"

#include "ik_effector_template.h"


void IKEffectorTemplate::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_target_node"), &IKEffectorTemplate::get_target_node);
	ClassDB::bind_method(D_METHOD("set_target_node", "target_node"), &IKEffectorTemplate::set_target_node);

	ClassDB::bind_method(D_METHOD("get_depth_falloff"), &IKEffectorTemplate::get_depth_falloff);
	ClassDB::bind_method(D_METHOD("set_depth_falloff", "depth_falloff"), &IKEffectorTemplate::set_depth_falloff);

	ClassDB::bind_method(D_METHOD("get_target_node_rotation"), &IKEffectorTemplate::get_target_node_rotation);
	ClassDB::bind_method(D_METHOD("set_target_node_rotation", "enabled"), &IKEffectorTemplate::set_target_node_rotation);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth_falloff"), "set_depth_falloff", "get_depth_falloff");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node"), "set_target_node", "get_target_node");
}

NodePath IKEffectorTemplate::get_target_node() const {
	return target_node;
}

void IKEffectorTemplate::set_target_node(NodePath p_node_path) {
	target_node = p_node_path;
}

float IKEffectorTemplate::get_depth_falloff() const {
	return depth_falloff;
}

void IKEffectorTemplate::set_depth_falloff(float p_depth_falloff) {
	depth_falloff = p_depth_falloff;
}

bool IKEffectorTemplate::get_target_node_rotation() const {
	return is_target_node_rotation;
}

void IKEffectorTemplate::set_target_node_rotation(bool p_target_node_rotation) {
	is_target_node_rotation = p_target_node_rotation;
}

IKEffectorTemplate::IKEffectorTemplate() {
}
