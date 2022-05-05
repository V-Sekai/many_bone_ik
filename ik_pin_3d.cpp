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

void IKEffector3D::set_target_node(Node *p_skeleton, const NodePath &p_target_node_path) {
	target_node = p_target_node_path;
}

NodePath IKEffector3D::get_target_node() const {
	return target_node;
}

void IKEffector3D::set_use_target_node_rotation(bool p_use) {
	use_target_node_rotation = p_use;
}

bool IKEffector3D::get_use_target_node_rotation() const {
	return use_target_node_rotation;
}

Ref<IKBone3D> IKEffector3D::get_shadow_bone() const {
	return for_bone;
}

bool IKEffector3D::is_following_translation_only() const {
	return false;
}

void IKEffector3D::update_goal_global_pose(Skeleton3D *p_skeleton) {
	target_global_pose = for_bone->get_global_pose();
	if (target_node == NodePath()) {
		return;
	}
	Node3D *current_target_node = cast_to<Node3D>(p_skeleton->get_node(target_node));
	if (!current_target_node) {
		return;
	}
	// TODO fire 2022-05-04 cache.
	target_global_pose = current_target_node->get_global_transform();
	target_global_pose = p_skeleton->world_transform_to_global_pose(target_global_pose);
}

Transform3D IKEffector3D::get_goal_global_pose() const {
	return target_global_pose;
}

void IKEffector3D::create_headings(Vector<real_t> &p_weights) {
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

void IKEffector3D::update_effector_target_headings(PackedVector3Array *p_headings,
		int32_t &p_index, Ref<IKBone3D> p_for_bone, Vector<real_t> *p_weights) const {
	ERR_FAIL_NULL(p_headings);

	Vector3 bone_origin = p_for_bone->get_global_pose().origin;
	// The global transform becomes global pose becomes current bone space.
	// Vector3(0.f, 0.f, 0.f) is the current bone's origin.

	p_headings->write[p_index] = target_global_pose.origin - bone_origin;
	p_index++;
	{
		real_t w = p_weights->write[p_index];
		w = MAX(w, 1.0f);
		p_headings->write[p_index] = (target_global_pose.basis.get_column(Vector3::AXIS_X) + target_global_pose.origin) - bone_origin;
		p_headings->write[p_index] *= Vector3(w, w, w);
		p_headings->write[p_index + 1] = (target_global_pose.origin - target_global_pose.basis.get_column(Vector3::AXIS_X)) - bone_origin;
		p_headings->write[p_index + 1] *= Vector3(w, w, w);
		p_index += 2;
	}
	{
		real_t w = p_weights->write[p_index];
		w = MAX(w, 1.0f);
		p_headings->write[p_index] = (target_global_pose.basis.get_column(Vector3::AXIS_Y) + target_global_pose.origin) - bone_origin;
		p_headings->write[p_index] *= Vector3(w, w, w);
		p_headings->write[p_index + 1] = (target_global_pose.origin - target_global_pose.basis.get_column(Vector3::AXIS_Y)) - bone_origin;
		p_headings->write[p_index + 1] *= Vector3(w, w, w);
		p_index += 2;
	}
	{
		real_t w = p_weights->write[p_index];
		w = MAX(w, 1.0f);
		p_headings->write[p_index] = (target_global_pose.basis.get_column(Vector3::AXIS_Z) + target_global_pose.origin) - bone_origin;
		p_headings->write[p_index] *= Vector3(w, w, w);
		p_headings->write[p_index + 1] = (target_global_pose.origin - target_global_pose.basis.get_column(Vector3::AXIS_Z)) - bone_origin;
		p_headings->write[p_index + 1] *= Vector3(w, w, w);
		p_index += 2;
	}
}

void IKEffector3D::update_effector_tip_headings(PackedVector3Array *p_headings, int32_t &p_index, Ref<IKBone3D> p_for_bone) const {
	ERR_FAIL_NULL(p_headings);
	Transform3D tip_xform = for_bone->get_global_pose();
	Basis tip_basis = tip_xform.basis;
	Vector3 bone_origin = p_for_bone->get_global_pose().origin;
	p_headings->write[p_index] = tip_xform.origin - bone_origin;
	// Multiply the target basis vectors by the distance to the current bone origin before adding them to the target origin.
	// The scaling amount we use is linear with distance and seems to work pretty well.
	// Haven't considered what would be the most mathematically rigorous scaling function.
	// Probably something like d / (4 pi r^2).
	double scale_by = MAX(1.0f, target_global_pose.origin.distance_to(bone_origin));
	// The global transform becomes global pose becomes current bone space.
	// In orientation of the skeleton global pose
	// The translation is in skeleton tip global pose - current bone global pose -
	// Vector3(0.f, 0.f, 0.f) is the current bone's origin.
	p_index++;
	{
		float distance = tip_basis.xform(Vector3(1.0f, 0.0f, 0.0f)).distance_to(tip_basis.get_column(Vector3::AXIS_X));
		p_headings->write[p_index] = ((tip_basis.get_column(Vector3::AXIS_X) * scale_by) + tip_xform.origin) - bone_origin;
		p_headings->write[p_index + 1] = (tip_xform.origin - (tip_xform.basis.get_column(Vector3::AXIS_X) * scale_by)) - bone_origin;
		p_index += 2;
	}
	{
		float distance = tip_basis.xform(Vector3(0.0f, 1.0f, 0.0f)).distance_to(tip_basis.get_column(Vector3::AXIS_Y));
		p_headings->write[p_index] = ((tip_basis.get_column(Vector3::AXIS_Y) * scale_by) + tip_xform.origin) - bone_origin;
		p_headings->write[p_index + 1] = (tip_xform.origin - (tip_xform.basis.get_column(Vector3::AXIS_Y) * scale_by)) - bone_origin;
		p_index += 2;
	}
	{
		float distance = tip_basis.xform(Vector3(0.0f, 0.0f, 1.0f)).distance_to(tip_basis.get_column(Vector3::AXIS_Z));
		p_headings->write[p_index] = ((tip_basis.get_column(Vector3::AXIS_Z) * scale_by) + tip_xform.origin) - bone_origin;
		p_headings->write[p_index + 1] = (tip_xform.origin - (tip_xform.basis.get_column(Vector3::AXIS_Z) * scale_by)) - bone_origin;
		p_index += 2;
	}
}

void IKEffector3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "skeleton", "node"),
			&IKEffector3D::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&IKEffector3D::get_target_node);
	ClassDB::bind_method(D_METHOD("set_depth_falloff", "amount"),
			&IKEffector3D::set_depth_falloff);
	ClassDB::bind_method(D_METHOD("get_depth_falloff"),
			&IKEffector3D::get_depth_falloff);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth_falloff"), "set_depth_falloff", "get_depth_falloff");
}

IKEffector3D::IKEffector3D(const Ref<IKBone3D> &p_current_bone) {
	for_bone = p_current_bone;
}

void IKEffector3D::set_depth_falloff(float p_depth_falloff) {
	depth_falloff = p_depth_falloff;
}

float IKEffector3D::get_depth_falloff() const {
	return depth_falloff;
}
