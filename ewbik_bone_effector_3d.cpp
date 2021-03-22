/*************************************************************************/
/*  ewbik_bone_effector_3d.cpp                                           */
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

#include "ewbik_bone_effector_3d.h"

void EWBIKBoneEffector3D::set_target_transform(const Transform &p_target_transform) {
	target_transform = p_target_transform;
}

Transform EWBIKBoneEffector3D::get_target_transform() const {
	return target_transform;
}

void EWBIKBoneEffector3D::set_target_node(const NodePath &p_target_node_path) {
	target_nodepath = p_target_node_path;
}

NodePath EWBIKBoneEffector3D::get_target_node() const {
	return target_nodepath;
}

void EWBIKBoneEffector3D::set_use_target_node_rotation(bool p_use) {
	use_target_node_rotation = p_use;
}

bool EWBIKBoneEffector3D::get_use_target_node_rotation() const {
	return use_target_node_rotation;
}

bool EWBIKBoneEffector3D::is_following_translation_only() const {
	return !(follow_x || follow_y || follow_z);
}

void EWBIKBoneEffector3D::update_goal_transform(Skeleton3D *p_skeleton) {
	goal_transform = Transform();
	Node *node = p_skeleton->get_node_or_null(target_nodepath);
	if (node) {
		Node3D *target_node = Object::cast_to<Node3D>(node);
		if (use_target_node_rotation) {
			goal_transform = p_skeleton->get_global_transform().affine_inverse() * target_node->get_global_transform();
		} else {
			goal_transform = Transform(Basis(), p_skeleton->to_local(target_node->get_global_transform().origin));
		}
	}
	goal_transform = target_transform * goal_transform;
}

void EWBIKBoneEffector3D::update_priorities() {
	follow_x = priority.x > 0.0;
	follow_y = priority.y > 0.0;
	follow_z = priority.z > 0.0;

	num_headings = 1;
	if (follow_x) {
		num_headings += 2;
	}
	if (follow_y) {
		num_headings += 2;
	}
	if (follow_z) {
		num_headings += 2;
	}
}

void EWBIKBoneEffector3D::update_target_headings(Skeleton3D *p_skeleton, PackedVector3Array &p_headings, Vector<real_t> &p_weights) {
	PackedVector3Array target_headings;
	target_headings.resize(num_headings);
	Vector3 origin = p_skeleton->get_bone_pose(for_bone->get_bone_id()).origin;
	int32_t index = 0;
	target_headings.write[index] = goal_transform.origin - origin;
	index++;

	if (follow_x) {
		Vector3 v = Vector3(weight * priority.x, 0.0, 0.0);
		target_headings.write[index] = goal_transform.xform(v) - origin;
		target_headings.write[index+1] = goal_transform.xform(-v) - origin;
		index += 2;
	}

	if (follow_y) {
		Vector3 v = Vector3(0.0, weight * priority.y, 0.0);
		target_headings.write[index] = goal_transform.xform(v) - origin;
		target_headings.write[index+1] = goal_transform.xform(-v) - origin;
		index += 2;
	}

	if (follow_z) {
		Vector3 v = Vector3(0.0, 0.0, weight * priority.z);
		target_headings.write[index] = goal_transform.xform(v) - origin;
		target_headings.write[index+1] = goal_transform.xform(-v) - origin;
	}

	p_headings.append_array(target_headings);

	Vector<real_t> weights;
	weights.resize(num_headings);
	for (int32_t i = 0; i < num_headings; i++) {
		weights.write[i] = weight;
	}

	p_weights.append_array(weights);
}

void EWBIKBoneEffector3D::update_tip_headings(Skeleton3D *p_skeleton, PackedVector3Array &p_headings, int32_t &p_index) {
	PackedVector3Array tip_headings;
	tip_headings.resize(num_headings);
	Vector3 origin = p_skeleton->get_bone_pose(for_bone->get_bone_id()).origin;
	Transform tip_xform = for_bone->get_transform();
	real_t scale_by = origin.distance_to(goal_transform.origin);
	tip_headings.write[p_index] = tip_xform.origin - origin;
	p_index++;

	if (follow_x) {
		Vector3 v = Vector3(scale_by, 0.0, 0.0);
		tip_headings.write[p_index] = tip_xform.xform(v) - origin;
		tip_headings.write[p_index+1] = tip_xform.xform(-v) - origin;
		p_index += 2;
	}

	if (follow_y) {
		Vector3 v = Vector3(0.0, scale_by, 0.0);
		tip_headings.write[p_index] = tip_xform.xform(v) - origin;
		tip_headings.write[p_index+1] = tip_xform.xform(-v) - origin;
		p_index += 2;
	}

	if (follow_z) {
		Vector3 v = Vector3(0.0, 0.0, scale_by);
		tip_headings.write[p_index] = tip_xform.xform(v) - origin;
		tip_headings.write[p_index+1] = tip_xform.xform(-v) - origin;
		p_index += 2;
	}

	p_headings.append_array(tip_headings);
}

void EWBIKBoneEffector3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_transform", "transform"),
			&EWBIKBoneEffector3D::set_target_transform);
	ClassDB::bind_method(D_METHOD("get_target_transform"),
			&EWBIKBoneEffector3D::get_target_transform);

	ClassDB::bind_method(D_METHOD("set_target_node", "node"),
			&EWBIKBoneEffector3D::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&EWBIKBoneEffector3D::get_target_node);
}

EWBIKBoneEffector3D::EWBIKBoneEffector3D(const Ref<EWBIKShadowBone3D> &p_for_bone) {
	for_bone = p_for_bone;
	update_priorities();
}
