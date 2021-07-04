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
#include "math/ik_transform.h"

void IKEffector3D::set_target_node(const NodePath &p_target_node_path, Node *p_skeleton) {
	target_node = p_target_node_path;
	update_target_cache(p_skeleton);
}

NodePath IKEffector3D::get_target_node() const {
	return target_node;
}

Transform3D IKEffector3D::get_goal_transform() const {
	return goal_transform;
}

Ref<IKBone3D> IKEffector3D::get_shadow_bone() const {
	return for_bone;
}

bool IKEffector3D::is_following_translation_only() const {
	return !(get_follow_x() || get_follow_y() || get_follow_z());
}

void IKEffector3D::update_goal_transform(Skeleton3D *p_skeleton) {
	goal_transform = Transform3D();
	if (target_node_reference == nullptr) {
		target_node_reference = Object::cast_to<Node3D>(ObjectDB::get_instance(target_node_cache));
	}
	if (!target_node_reference) {
		goal_transform = for_bone->get_global_transform();
		return;
	}
	if (!target_node_reference->is_class("Node3D")) {
		return;
	}
	Node3D *target_node = Object::cast_to<Node3D>(target_node_reference);
	Transform3D node_xform = target_node->get_global_transform();
	goal_transform = node_xform;
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

void IKEffector3D::update_effector_target_headings(PackedVector3Array *p_headings, int32_t &p_index,
		Vector<real_t> *p_weights) const {
	ERR_FAIL_NULL(p_headings);
	p_headings->write[p_index] = goal_transform.origin;
	p_index++;

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

void IKEffector3D::update_effector_tip_headings(Ref<IKBone3D> p_for_bone, PackedVector3Array *p_headings, int32_t &p_index) const {
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
	ClassDB::bind_method(D_METHOD("set_target_node", "node"),
			&IKEffector3D::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&IKEffector3D::get_target_node);
}

IKEffector3D::IKEffector3D(const Ref<IKBone3D> &p_for_bone) {
	for_bone = p_for_bone;
	update_priorities();
}

void IKEffector3D::update_target_cache(Node *p_skeleton) {
	ERR_FAIL_NULL(p_skeleton);
	if (!p_skeleton->is_inside_tree()) {
		return;
	}
	if (!p_skeleton->has_node(target_node)) {
		return;
	}
	target_node_cache = ObjectID();
	Node *node = p_skeleton->get_node(target_node);
	ERR_FAIL_COND_MSG(!node || p_skeleton == node,
			"Cannot update target cache: node is this modification's skeleton or cannot be found!");
	ERR_FAIL_COND_MSG(!node->is_inside_tree(),
			"Cannot update target cache: node is not in scene tree!");
	target_node_cache = node->get_instance_id();
}
