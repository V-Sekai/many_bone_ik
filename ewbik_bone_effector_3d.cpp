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

Transform EWBIKBoneEffector3D::get_goal_transform() const {
	return goal_transform;
}

Ref<EWBIKShadowBone3D> EWBIKBoneEffector3D::get_shadow_bone() const {
	return for_bone;
}

bool EWBIKBoneEffector3D::is_following_translation_only() const {
	return !(follow_x || follow_y || follow_z);
}

void EWBIKBoneEffector3D::update_goal_transform(Skeleton3D *p_skeleton) {
	goal_transform = Transform();
	Node *node = p_skeleton->get_node_or_null(target_nodepath);
	if (node && node->is_class("Node3D")) {
		Node3D *target_node = Object::cast_to<Node3D>(node);
		if (use_target_node_rotation) {
			goal_transform = p_skeleton->get_global_transform().affine_inverse() * target_node->get_global_transform();
		} else {
			goal_transform = Transform(Basis(), p_skeleton->to_local(target_node->get_global_transform().origin));
		}
		goal_transform = target_transform * goal_transform;
	} else {
		goal_transform = for_bone->get_global_transform() * target_transform;
	}
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


void EWBIKBoneEffector3D::create_weights(Vector<real_t> &p_weights, real_t p_falloff) const {
	Vector<real_t> weights;
	weights.resize(num_headings);
	int32_t index = 0;
	weights.write[index] = weight * p_falloff;
	index++;

	if (follow_x) {
		weights.write[index] = weight * priority.x * p_falloff;
		weights.write[index+1] = weight * priority.x * p_falloff;
		index += 2;
	}

	if (follow_y) {
		weights.write[index] = weight * priority.y * p_falloff;
		weights.write[index+1] = weight * priority.y * p_falloff;
		index += 2;
	}

	if (follow_z) {
		weights.write[index] = weight * priority.z * p_falloff;
		weights.write[index+1] = weight * priority.z * p_falloff;
	}
	p_weights.append_array(weights);
}

void EWBIKBoneEffector3D::update_target_headings(Ref<EWBIKShadowBone3D> p_for_bone, PackedVector3Array &p_headings, int32_t &p_index, Vector<real_t> &p_weights) const {
	Vector3 origin = p_for_bone->get_global_transform().origin;
	p_headings.write[p_index] = goal_transform.origin - origin;
	p_index++;

	if (follow_x) {
		float_t w = MAX(p_weights[p_index], MIN_SCALE);
		Vector3 v = Vector3(w, 0.0, 0.0);
		p_headings.write[p_index] = goal_transform.xform(v) - origin;
		p_headings.write[p_index+1] = goal_transform.xform(-v) - origin;
		p_index += 2;
	}

	if (follow_y) {
		float_t w = MAX(p_weights[p_index], MIN_SCALE);
		Vector3 v = Vector3(0.0, w, 0.0);
		p_headings.write[p_index] = goal_transform.xform(v) - origin;
		p_headings.write[p_index+1] = goal_transform.xform(-v) - origin;
		p_index += 2;
	}

	if (follow_z) {
		float_t w = MAX(p_weights[p_index], MIN_SCALE);
		Vector3 v = Vector3(0.0, 0.0, w);
		p_headings.write[p_index] = goal_transform.xform(v) - origin;
		p_headings.write[p_index+1] = goal_transform.xform(-v) - origin;
		p_index += 2;
	}
}

void EWBIKBoneEffector3D::update_tip_headings(Ref<EWBIKShadowBone3D> p_for_bone, PackedVector3Array &p_headings, int32_t &p_index) const {
	Vector3 origin = p_for_bone->get_global_transform().origin;
	Transform tip_xform = for_bone->get_transform();
	real_t scale_by = MAX(origin.distance_to(goal_transform.origin), MIN_SCALE);
	p_headings.write[p_index] = tip_xform.origin - origin;
	p_index++;

	if (follow_x) {
		Vector3 v = Vector3(scale_by, 0.0, 0.0);
		p_headings.write[p_index] = tip_xform.xform(v) - origin;
		p_headings.write[p_index+1] = tip_xform.xform(-v) - origin;
		p_index += 2;
	}

	if (follow_y) {
		Vector3 v = Vector3(0.0, scale_by, 0.0);
		p_headings.write[p_index] = tip_xform.xform(v) - origin;
		p_headings.write[p_index+1] = tip_xform.xform(-v) - origin;
		p_index += 2;
	}

	if (follow_z) {
		Vector3 v = Vector3(0.0, 0.0, scale_by);
		p_headings.write[p_index] = tip_xform.xform(v) - origin;
		p_headings.write[p_index+1] = tip_xform.xform(-v) - origin;
		p_index += 2;
	}
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
