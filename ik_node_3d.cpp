#include "ik_node_3d.h"

void IKNode3D::set_height(float p_height) {
	height = p_height;
}

float IKNode3D::get_height() const {
	return height;
}

void IKNode3D::set_stiffness(float p_stiffness) {
	stiffness = p_stiffness;
}

float IKNode3D::get_stiffness() const {
	return stiffness;
}

IKBasis IKNode3D::get_local() const {
	return pose_local;
}

IKBasis IKNode3D::get_global() const {
	return pose_global;
}

Transform IKNode3D::get_global_transform() const {
	Transform xform;
	IKBasis basis = pose_global;
	xform.origin = basis.get_origin();
	xform.basis = basis.get_rotation();
	return xform;
}

void IKNode3D::set_local(IKBasis p_local) {
	pose_local = p_local;
}

void IKNode3D::set_parent(int32_t p_parent) {
	parent = p_parent;
}
