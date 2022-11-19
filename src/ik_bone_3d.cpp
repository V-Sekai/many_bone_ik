/*************************************************************************/
/*  ik_bone_3d.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "ik_ewbik.h"
#include "math/ik_node_3d.h"

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
		constraint_transform->set_parent(godot_skeleton_aligned_transform->get_parent());
	}
}

void IKBone3D::update_default_bone_direction_transform(Skeleton3D *p_skeleton) {
	Vector3 child_centroid;
	if (children.is_empty()) {
		PackedInt32Array bone_children = p_skeleton->get_bone_children(bone_id);
		for (BoneId child_bone_idx : bone_children) {
			child_centroid += p_skeleton->get_bone_global_pose(child_bone_idx).origin;
		}
		child_centroid /= bone_children.size();
	} else {
		for (Ref<IKBone3D> ik_bone : children) {
			child_centroid += ik_bone->get_ik_transform()->get_global_transform().origin;
		}
		child_centroid /= children.size();
	}
	Vector3 godot_bone_origin = godot_skeleton_aligned_transform->get_global_transform().origin;
	child_centroid -= godot_bone_origin;
	if (Math::is_zero_approx(child_centroid.length_squared())) {
		Vector3 parent_y_direction =   parent->get_ik_transform()->get_global_transform().xform(Vector3(0, 1, 0)) - parent->get_ik_transform()->get_global_transform().origin;
		child_centroid = parent->get_bone_direction_transform()->get_global_transform().xform(Vector3(0, 1, 0)) - parent->get_bone_direction_transform()->get_global_transform().origin;
	}
	if (!Math::is_zero_approx(child_centroid.length_squared()) && (children.size() || p_skeleton->get_bone_children(bone_id).size())) {
		child_centroid.normalize();
		Vector3 bone_direction = bone_direction_transform->get_global_transform().xform(Vector3(0.0, 1.0, 0.0)) - godot_bone_origin;
		bone_direction.normalize();
		bone_direction_transform->rotate_local_with_global(Quaternion(child_centroid, bone_direction));
	}
}

void IKBone3D::update_default_constraint_transform() {
	Ref<IKBone3D> parent_bone = get_parent();
	if (parent_bone.is_valid()) {
		Transform3D parent_bone_direction = parent_bone->get_bone_direction_transform()->get_global_transform();
		parent_bone_direction.origin = get_bone_direction_transform()->get_global_transform().origin;
		constraint_transform->set_global_transform(parent_bone_direction);
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
	godot_skeleton_aligned_transform->set_transform(p_transform);
}

Transform3D IKBone3D::get_pose() const {
	return godot_skeleton_aligned_transform->get_transform();
}

void IKBone3D::set_global_pose(const Transform3D &p_transform) {
	godot_skeleton_aligned_transform->set_global_transform(p_transform);
	constraint_transform->local_transform.origin = godot_skeleton_aligned_transform->local_transform.origin;
	constraint_transform->_propagate_transform_changed();
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
	ClassDB::bind_method(D_METHOD("get_constraint_transform"), &IKBone3D::get_constraint_transform);
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
			const real_t weight = elem->get_weight();
			effector->set_weight(elem->get_weight());
			effector->set_weight(weight);
			effector->set_direction_priorities(elem->get_direction_priorities());
			break;
		}
	}
	bone_direction_transform->set_parent(godot_skeleton_aligned_transform);
}

float IKBone3D::get_cos_half_dampen() const {
	return cos_half_dampen;
}

void IKBone3D::set_cos_half_dampen(float p_cos_half_dampen) {
	cos_half_dampen = p_cos_half_dampen;
}

Ref<IKKusudama> IKBone3D::get_constraint() const {
	return constraint;
}

void IKBone3D::add_constraint(Ref<IKKusudama> p_constraint) {
	constraint = p_constraint;
}

Ref<IKNode3D> IKBone3D::get_ik_transform() {
	return godot_skeleton_aligned_transform;
}

Ref<IKNode3D> IKBone3D::get_constraint_transform() {
	return constraint_transform;
}

void IKBone3D::set_constraint_transform(Ref<IKNode3D> p_transform) {
	constraint_transform = p_transform;
}

void IKBone3D::set_bone_direction_transform(Ref<IKNode3D> p_bone_direction) {
	bone_direction_transform = p_bone_direction;
}

Ref<IKNode3D> IKBone3D::get_bone_direction_transform() {
	return bone_direction_transform;
}
void IKBone3D::set_stiffness(float p_stiffness) {
	stiffness = p_stiffness;
}
float IKBone3D::get_stiffness() const {
	return stiffness;
}
