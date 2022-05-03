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

void IKBone3D::set_bone_id(BoneId p_bone_id, Skeleton3D *p_skeleton) {
	bone_id = p_bone_id;
}

BoneId IKBone3D::get_bone_id() const {
	return bone_id;
}

void IKBone3D::set_parent(const Ref<IKBone3D> &p_parent) {
	parent = p_parent;
	if (parent.is_valid()) {
		parent->children.push_back(this);
		xform.set_parent(&parent->xform);
	}
}

Vector3 IKBone3D::get_translation_delta() const {
	return translation_delta;
}

Ref<IKBone3D> IKBone3D::get_parent() const {
	return parent;
}

void IKBone3D::set_pin(const Ref<IKManipulator3D> &p_pin) {
	pin = p_pin;
}

Ref<IKManipulator3D> IKBone3D::get_pin() const {
	return pin;
}

void IKBone3D::set_pose(const Transform3D &p_transform) {
	xform.set_transform(p_transform);
}

Transform3D IKBone3D::get_pose() const {
	return xform.get_transform();
}

void IKBone3D::set_global_pose(const Transform3D &p_transform) {
	xform.set_global_transform(p_transform);
}

Transform3D IKBone3D::get_global_pose() const {
	return xform.get_global_transform();
}

void IKBone3D::set_global_pose_rot_delta(const Basis &p_rot) {
	rot_delta = p_rot;
	Transform3D xform = get_global_pose() * Transform3D(p_rot, translation_delta);
	xform.orthogonalize();
	set_global_pose(xform);
}

void IKBone3D::set_initial_pose(Skeleton3D *p_skeleton) {
	const Vector3 &position = p_skeleton->get_bone_pose_position(bone_id);
	const Quaternion &rot = p_skeleton->get_bone_pose_rotation(bone_id);
	const Vector3 &scale = p_skeleton->get_bone_pose_scale(bone_id);
	set_pose(Transform3D(Basis(rot, scale), position));
}

void IKBone3D::set_skeleton_bone_pose(Skeleton3D *p_skeleton, real_t p_strength) {
	Transform3D custom = get_pose();
	p_skeleton->set_bone_pose_position(bone_id, custom.origin);
	p_skeleton->set_bone_pose_rotation(bone_id, custom.basis.get_rotation_quaternion());
	p_skeleton->set_bone_pose_scale(bone_id, custom.basis.get_scale());
}

void IKBone3D::create_pin() {
	pin = Ref<IKManipulator3D>(memnew(IKManipulator3D(this)));
}

bool IKBone3D::is_pin() const {
	return pin.is_valid();
}

void IKBone3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_pin"), &IKBone3D::get_pin);
	ClassDB::bind_method(D_METHOD("set_pin", "pin"), &IKBone3D::set_pin);
	ClassDB::bind_method(D_METHOD("is_pinned"), &IKBone3D::is_pin);
}

IKBone3D::IKBone3D(String p_bone, Skeleton3D *p_skeleton, const Ref<IKBone3D> &p_parent, float p_default_dampening) :
		default_dampening(p_default_dampening) {
	set_name(p_bone);
	bone_id = p_skeleton->find_bone(p_bone);
	set_parent(p_parent);
}

float IKBone3D::get_cos_half_dampen() const {
	return cos_half_dampen;
}

void IKBone3D::set_cos_half_dampen(float p_cos_half_dampen) {
	cos_half_dampen = p_cos_half_dampen;
}

void IKBone3D::set_global_pose_translation_delta(Vector3 p_translation_delta) {
	translation_delta = p_translation_delta;
	Transform3D xform = get_global_pose();
	xform.origin += p_translation_delta;
	set_global_pose(xform);
}