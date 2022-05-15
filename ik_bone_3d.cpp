/*************************************************************************/
/*  ewbik_shadow_bone_3d.cpp                                             */
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

#include "ik_bone_3d.h"

#include "math/ik_transform.h"
#include "skeleton_modification_3d_ewbik.h"

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
		xform->set_parent(parent->xform);
		constraint_transform->set_parent(xform->get_parent());
	}
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
	xform->set_transform(p_transform);
}

Transform3D IKBone3D::get_pose() const {
	return xform->get_transform();
}

void IKBone3D::set_global_pose(const Transform3D &p_transform) {
	xform->set_global_transform(p_transform);
	constraint_transform->local_transform.origin = xform->local_transform.origin;
	constraint_transform->_propagate_transform_changed();
	// constraint_transform->set_local_transform(constraint_transform->get_local_transform().basis, xform->get_global_transform().origin)
}

Transform3D IKBone3D::get_global_pose() const {
	return xform->get_global_transform();
}

void IKBone3D::set_initial_pose(Skeleton3D *p_skeleton) {
	ERR_FAIL_NULL(p_skeleton);
	if (bone_id == -1) {
		return;
	}
	Transform3D xform = p_skeleton->get_bone_global_pose(bone_id);
	set_global_pose(xform);
}

void IKBone3D::set_skeleton_bone_pose(Skeleton3D *p_skeleton, real_t p_strength) {
	ERR_FAIL_NULL(p_skeleton);
	Transform3D custom = get_pose();
	p_skeleton->set_bone_pose_position(bone_id, custom.origin);
	p_skeleton->set_bone_pose_rotation(bone_id, custom.basis.get_rotation_quaternion());
	p_skeleton->set_bone_pose_scale(bone_id, custom.basis.get_scale());
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
}

IKBone3D::IKBone3D(StringName p_bone, Skeleton3D *p_skeleton, const Ref<IKBone3D> &p_parent, Vector<Ref<IKEffectorTemplate>> &p_pins, float p_default_dampening) {
	ERR_FAIL_NULL(p_skeleton);
	default_dampening = p_default_dampening;
	cos_half_dampen = default_dampening / real_t(2.0);
	set_name(p_bone);
	bone_id = p_skeleton->find_bone(p_bone);
	if (p_parent.is_valid()) {
		set_parent(p_parent);
	}
	for (Ref<IKEffectorTemplate> elem : p_pins) {
		if (elem.is_null()) {
			continue;
		}
		if (elem->get_name() == p_bone) {
			create_pin();
			Ref<IKEffector3D> effector = get_pin();
			effector->set_target_node(p_skeleton, elem->get_target_node());
			effector->set_depth_falloff(elem->get_depth_falloff());
			effector->set_target_node_rotation(elem->get_target_node_rotation());
			break;
		}
	}
	bone_direction_transform->set_parent(xform);
	constraint.instantiate();
	constraint->add_limit_cone_at_index(0, Vector3(0.f, 1.f, 0.0f), (3 * Math_PI) / 4.0f);
	constraint->set_axial_limits(0.0f, Math_PI);
	constraint->enable_orientational_limits();
	constraint->enable_axial_limits();
}

float IKBone3D::get_cos_half_dampen() const {
	return cos_half_dampen;
}

void IKBone3D::set_cos_half_dampen(float p_cos_half_dampen) {
	cos_half_dampen = p_cos_half_dampen;
}

Ref<IKKusudama> IKBone3D::getConstraint() const {
	return constraint;
}

void IKBone3D::updateCosDampening() {
}

void IKBone3D::addConstraint(Ref<IKKusudama> p_constraint) {
}

Ref<IKTransform3D> IKBone3D::get_ik_transform() {
	return xform;
}
Ref<IKTransform3D> IKBone3D::get_constraint_transform() {
	return constraint_transform;
}