/*************************************************************************/
/*  ik_effector_3d.cpp                                           */
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

#include "ik_effector_3d.h"

void IKEffector3D::set_target_transform(const Transform3D &p_target_transform) {
	target_transform = p_target_transform;
}

Transform3D IKEffector3D::get_target_transform() const {
	return target_transform;
}

void IKEffector3D::set_target_node(const NodePath &p_target_node_path) {
	target_nodepath = p_target_node_path;
}

NodePath IKEffector3D::get_target_node() const {
	return target_nodepath;
}

void IKEffector3D::set_use_target_node_rotation(bool p_use) {
	use_target_node_rotation = p_use;
}

bool IKEffector3D::get_use_target_node_rotation() const {
	return use_target_node_rotation;
}

Transform3D IKEffector3D::get_goal_transform() const {
	return goal_transform;
}

bool IKEffector3D::is_node_xform_changed(Skeleton3D *p_skeleton) const {
	Node *node = p_skeleton->get_node_or_null(target_nodepath);
	if (node && node->is_class("Node3D")) {
		Node3D *target_node = Object::cast_to<Node3D>(node);
		return prev_node_xform != target_node->get_global_transform();
	}
	return false;
}

Ref<IKBone3D> IKEffector3D::get_shadow_bone() const {
	return for_bone;
}

bool IKEffector3D::is_following_translation_only() const {
	return !(get_follow_x() || get_follow_y() || get_follow_z());
}

void IKEffector3D::update_goal_transform(Skeleton3D *p_skeleton) {
	goal_transform = Transform3D();
	Node *node = p_skeleton->get_node_or_null(target_nodepath);
	if (node && node->is_class("Node3D")) {
		Node3D *target_node = Object::cast_to<Node3D>(node);
		Transform3D node_xform = target_node->get_global_transform();
		if (use_target_node_rotation) {
			goal_transform = p_skeleton->get_global_transform().affine_inverse() * node_xform;
		} else {
			goal_transform = Transform3D(Basis(), p_skeleton->to_local(node_xform.origin));
		}
		prev_node_xform = node_xform;
		goal_transform = target_transform * goal_transform;
	} else {
		goal_transform = for_bone->get_global_transform() * target_transform;
	}
}

void IKEffector3D::update_priorities() {
	follow_x = priority.x > 0.0;
	follow_y = priority.y > 0.0;
	follow_z = priority.z > 0.0;

	num_headings = 1;
	if (get_follow_x()) {
		num_headings += 2;
	}
	if (get_follow_y()) {
		num_headings += 2;
	}
	if (get_follow_z()) {
		num_headings += 2;
	}
}

void IKEffector3D::create_headings(const Vector<real_t> &p_weights) {
	/**
	 * Weights are given from the parent chain. The last two weights should
	 * always correspond to this effector weights. In the parent only the origin
	 * is considered for rotation, but here the last two headings must be replaced
	 * by the corresponding number of "axis-orientation" headings.
	*/
	int32_t nw = p_weights.size();
	int32_t nheadings = nw + num_headings;
	heading_weights.resize(nheadings);
	tip_headings.resize(nheadings);
	target_headings.resize(nheadings);

	for (int32_t i_w = 0; i_w < nw; i_w++) {
		heading_weights.write[i_w] = p_weights[i_w];
	}

	int32_t index = 0;
	heading_weights.write[nw + index] = weight;
	index++;

	if (get_follow_x()) {
		heading_weights.write[nw + index] = weight * priority.x;
		heading_weights.write[nw + index + 1] = weight * priority.x;
		index += 2;
	}

	if (get_follow_y()) {
		heading_weights.write[nw + index] = weight * priority.y;
		heading_weights.write[nw + index + 1] = weight * priority.y;
		index += 2;
	}

	if (get_follow_z()) {
		heading_weights.write[nw + index] = weight * priority.z;
		heading_weights.write[nw + index + 1] = weight * priority.z;
	}
}

void IKEffector3D::update_target_headings(Ref<IKBone3D> p_for_bone, PackedVector3Array *p_headings, int32_t &p_index,
		Vector<real_t> *p_weights) const {
	ERR_FAIL_NULL(p_headings);
	p_headings->write[p_index] = goal_transform.origin;
	p_index++;

	if (p_for_bone->get_parent().is_null()) {
		return;
	}

	if (get_follow_x()) {
		real_t w = p_weights->write[p_index];
		Vector3 v = Vector3(w, 0.0, 0.0);
		p_headings->write[p_index] = goal_transform.xform(v);
		p_headings->write[p_index + 1] = goal_transform.xform(-v);
		p_index += 2;
	}

	if (get_follow_y()) {
		real_t w = p_weights->write[p_index];
		Vector3 v = Vector3(0.0, w, 0.0);
		p_headings->write[p_index] = goal_transform.xform(v);
		p_headings->write[p_index + 1] = goal_transform.xform(-v);
		p_index += 2;
	}

	if (get_follow_z()) {
		real_t w = p_weights->write[p_index];
		Vector3 v = Vector3(0.0, 0.0, w);
		p_headings->write[p_index] = goal_transform.xform(v);
		p_headings->write[p_index + 1] = goal_transform.xform(-v);
		p_index += 2;
	}
}

void IKEffector3D::update_tip_headings(Ref<IKBone3D> p_for_bone, PackedVector3Array *p_headings, int32_t &p_index) const {
	ERR_FAIL_NULL(p_headings);
	ERR_FAIL_NULL(p_for_bone);
	Transform3D tip_xform = for_bone->get_global_transform();
	p_headings->write[p_index] = tip_xform.origin;
	p_index++;
	real_t scale_by = 1.0f; //MAX(goal_transform.origin, MIN_SCALE);

	if (get_follow_x()) {
		Vector3 v = Vector3(scale_by, 0.0, 0.0);
		p_headings->write[p_index] = tip_xform.xform(v);
		p_headings->write[p_index + 1] = tip_xform.xform(-v);
		p_index += 2;
	}

	if (get_follow_y()) {
		Vector3 v = Vector3(0.0, scale_by, 0.0);
		p_headings->write[p_index] = tip_xform.xform(v);
		p_headings->write[p_index + 1] = tip_xform.xform(-v);
		p_index += 2;
	}

	if (get_follow_z()) {
		Vector3 v = Vector3(0.0, 0.0, scale_by);
		p_headings->write[p_index] = tip_xform.xform(v);
		p_headings->write[p_index + 1] = tip_xform.xform(-v);
		p_index += 2;
	}
}

void IKEffector3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_transform", "transform"),
			&IKEffector3D::set_target_transform);
	ClassDB::bind_method(D_METHOD("get_target_transform"),
			&IKEffector3D::get_target_transform);

	ClassDB::bind_method(D_METHOD("set_target_node", "node"),
			&IKEffector3D::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&IKEffector3D::get_target_node);
}

IKEffector3D::IKEffector3D(const Ref<IKBone3D> &p_for_bone) {
	for_bone = p_for_bone;
	update_priorities();
}
