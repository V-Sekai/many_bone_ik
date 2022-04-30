/*************************************************************************/
/*  ik_pin_3d.cpp                                                        */
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

#include "ik_pin_3d.h"
#include "math/ik_transform.h"

void IKPin3D::set_target_node(Node *p_skeleton, const NodePath &p_target_node_path) {
	target_node = p_target_node_path;
	update_target_cache(p_skeleton);
}

NodePath IKPin3D::get_target_node() const {
	return target_node;
}

void IKPin3D::set_use_target_node_rotation(bool p_use) {
	use_target_node_rotation = p_use;
}

bool IKPin3D::get_use_target_node_rotation() const {
	return use_target_node_rotation;
}

Ref<IKBone3D> IKPin3D::get_shadow_bone() const {
	return for_bone;
}

bool IKPin3D::is_following_translation_only() const {
	return false;
}

void IKPin3D::update_goal_global_pose(Skeleton3D *p_skeleton) {
	goal_global_pose = Transform3D();
	if (!target_node_reference) {
		target_node_reference = Object::cast_to<Node3D>(ObjectDB::get_instance(target_node_cache));
		goal_global_pose = for_bone->get_global_pose();
		if (!use_target_node_rotation) {
			goal_global_pose.basis = Basis();
		}
		return;
	}
	Node3D *target_node = Object::cast_to<Node3D>(target_node_reference);
	Transform3D node_xform = target_node->get_global_transform();
	goal_global_pose = p_skeleton->world_transform_to_global_pose(node_xform);
	if (!use_target_node_rotation) {
		goal_global_pose.basis = Basis();
	}
}

Transform3D IKPin3D::get_goal_global_pose() const {
	return goal_global_pose;
}

void IKPin3D::create_headings(const Vector<real_t> &p_weights) {
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
	{
		ERR_FAIL_INDEX(nw + index, heading_weights.size());
		heading_weights.write[nw + index] = weight;
		ERR_FAIL_INDEX(nw + index + 1, heading_weights.size());
		heading_weights.write[nw + index + 1] = weight;
		index += 2;
	}
	{
		ERR_FAIL_INDEX(nw + index, heading_weights.size());
		heading_weights.write[nw + index] = weight;
		ERR_FAIL_INDEX(nw + index + 1, heading_weights.size());
		heading_weights.write[nw + index + 1] = weight;
		index += 2;
	}
	{
		ERR_FAIL_INDEX(nw + index, heading_weights.size());
		heading_weights.write[nw + index] = weight;
		ERR_FAIL_INDEX(nw + index + 1, heading_weights.size());
		heading_weights.write[nw + index + 1] = weight;
	}
}

void IKPin3D::update_effector_target_headings(PackedVector3Array *p_headings, int32_t &p_index,
		Vector<real_t> *p_weights) const {
	ERR_FAIL_NULL(p_headings);
	float scale_by = goal_global_pose.origin.length();
	p_headings->write[p_index] = goal_global_pose.origin;
	p_index++;
	{
		real_t w = p_weights->write[p_index];
		p_headings->write[p_index] = goal_global_pose.origin + scale_by * goal_global_pose.xform(Vector3(1.0f, 0.0f, 0.0f));
		p_headings->write[p_index] *= Vector3(w, 1.0, 1.0);
		p_headings->write[p_index + 1] = goal_global_pose.origin - scale_by * goal_global_pose.xform(Vector3(1.0f, 0.0f, 0.0f));
		p_headings->write[p_index] *= Vector3(w, 1.0, 1.0);
		p_index += 2;
	}
	{
		real_t w = p_weights->write[p_index];
		p_headings->write[p_index] = goal_global_pose.origin + scale_by * goal_global_pose.xform(Vector3(0.0f, 1.0f, 0.0f));
		p_headings->write[p_index] *= Vector3(1.0, w, 1.0);
		p_headings->write[p_index + 1] = goal_global_pose.origin - scale_by * goal_global_pose.xform(Vector3(0.0f, 1.0f, 0.0f));
		p_headings->write[p_index] *= Vector3(1.0, w, 1.0);
		p_index += 2;
	}
	{
		real_t w = p_weights->write[p_index];
		p_headings->write[p_index] = goal_global_pose.origin + scale_by * goal_global_pose.xform(Vector3(0.0f, 0.0f, 1.0f));
		p_headings->write[p_index] *= Vector3(1.0, 1.0, w);
		p_headings->write[p_index + 1] = goal_global_pose.origin - scale_by * goal_global_pose.xform(Vector3(0.0f, 0.0f, 1.0f));
		p_headings->write[p_index] *= Vector3(1.0, 1.0, w);
		p_index += 2;
	}
}

void IKPin3D::update_effector_tip_headings(Ref<IKBone3D> p_current_bone, PackedVector3Array *p_headings, int32_t &p_index) const {
	ERR_FAIL_NULL(p_headings);
	ERR_FAIL_NULL(p_current_bone);
	Transform3D tip_xform = for_bone->get_global_pose();
	p_headings->write[p_index] = tip_xform.origin;
	p_index++;
	Vector3 v = tip_xform.xform(Vector3());
	float scale_by = tip_xform.origin.length();
	if (for_bone->get_parent().is_valid()) {
		scale_by = for_bone->get_parent()->get_global_pose().origin.distance_to(tip_xform.origin);
	}
	{
		p_headings->write[p_index] = tip_xform.origin + scale_by * tip_xform.xform(Vector3(1.0f, 0.0f, 0.0f));
		p_headings->write[p_index + 1] = tip_xform.origin - scale_by * tip_xform.xform(Vector3(1.0f, 0.0f, 0.0f));
		p_index += 2;
	}
	{
		p_headings->write[p_index] = tip_xform.origin + scale_by * tip_xform.xform(Vector3(0.0f, 1.0f, 0.0f));
		p_headings->write[p_index + 1] = tip_xform.origin - scale_by * tip_xform.xform(Vector3(0.0f, 1.0f, 0.0f));
		p_index += 2;
	}
	{
		p_headings->write[p_index] = tip_xform.origin + scale_by * tip_xform.xform(Vector3(0.0f, 0.0f, 1.0f));
		p_headings->write[p_index + 1] = tip_xform.origin - scale_by * tip_xform.xform(Vector3(0.0f, 0.0f, 1.0f));
		p_index += 2;
	}
}

void IKPin3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "skeleton", "node"),
			&IKPin3D::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&IKPin3D::get_target_node);
	ClassDB::bind_method(D_METHOD("set_depth_falloff", "amount"),
			&IKPin3D::set_depth_falloff);
	ClassDB::bind_method(D_METHOD("get_depth_falloff"),
			&IKPin3D::get_depth_falloff);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth_falloff"), "set_depth_falloff", "get_depth_falloff");
}

IKPin3D::IKPin3D(const Ref<IKBone3D> &p_current_bone) {
	for_bone = p_current_bone;
}

void IKPin3D::update_target_cache(Node *p_skeleton) {
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

void IKPin3D::set_depth_falloff(float p_depth_falloff) {
	depth_falloff = p_depth_falloff;
}

float IKPin3D::get_depth_falloff() const {
	return depth_falloff;
}
