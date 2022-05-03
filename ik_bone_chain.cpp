/*************************************************************************/
/*  ik_bone_chain.cpp                                      */
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

#include "ik_bone_chain.h"
#include "scene/3d/skeleton_3d.h"

Ref<IKBone3D> IKBoneChain::get_root() const {
	return root;
}
Ref<IKBone3D> IKBoneChain::get_tip() const {
	return tip;
}

bool IKBoneChain::is_root_pinned() const {
	ERR_FAIL_NULL_V(root, false);
	return root->get_parent().is_null() && root->is_pin();
}

bool IKBoneChain::is_pin() const {
	ERR_FAIL_NULL_V(tip, false);
	return tip->is_pin();
}

Vector<Ref<IKBoneChain>> IKBoneChain::get_child_chains() const {
	return child_chains;
}

BoneId IKBoneChain::find_root_bone_id(BoneId p_bone) {
	BoneId root_id = p_bone;
	while (skeleton->get_bone_parent(root_id) != -1) {
		root_id = skeleton->get_bone_parent(root_id);
	}

	return root_id;
}

void IKBoneChain::generate_default_segments_from_root() {
	Ref<IKBone3D> temp_tip = root;
	while (true) {
		Vector<BoneId> children = skeleton->get_bone_children(temp_tip->get_bone_id());
		if (children.size() > 1) {
			tip = temp_tip;
			Ref<IKBoneChain> parent(this);
			for (int32_t child_i = 0; child_i < children.size(); child_i++) {
				BoneId child_bone = children[child_i];
				Ref<IKBoneChain> child_segment = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, child_bone, parent)));
				child_segment->generate_default_segments_from_root();
				child_chains.push_back(child_segment);
			}
			break;
		} else if (children.size() == 1) {
			BoneId bone_id = children[0];
			Ref<IKBone3D> next = Ref<IKBone3D>(memnew(IKBone3D(skeleton->get_bone_name(bone_id), skeleton, temp_tip)));
			temp_tip = next;
		} else {
			tip = temp_tip;
			tip->create_pin();
			break;
		}
	}
	set_name(vformat("IKBoneChain%sRoot%sTip", root->get_name(), tip->get_name()));
	bones.clear();
	set_bone_list(bones, false);
}

void IKBoneChain::set_bone_list(Vector<Ref<IKBone3D>> &p_list, bool p_recursive, bool p_debug_skeleton) const {
	if (p_recursive) {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			child_chains[child_i]->set_bone_list(p_list, p_recursive, p_debug_skeleton);
		}
	}
	Ref<IKBone3D> current_bone = tip;
	Vector<Ref<IKBone3D>> list;
	while (current_bone.is_valid()) {
		list.push_back(current_bone);
		if (current_bone == root) {
			break;
		}
		current_bone = current_bone->get_parent();
	}
	if (p_debug_skeleton) {
		for (int32_t name_i = 0; name_i < list.size(); name_i++) {
			BoneId bone = list[name_i]->get_bone_id();
			String bone_name = skeleton->get_bone_name(bone);
			String effector;
			if (list[name_i]->is_pin()) {
				effector += "Effector ";
			}
			String prefix;
			if (list[name_i] == root) {
				prefix += "(" + effector + "Root) ";
			} else if (list[name_i] == tip) {
				prefix += "(" + effector + "Tip) ";
			}
			print_line(prefix + bone_name);
		}
	}
	p_list.append_array(list);
}

void IKBoneChain::update_pinned_list() {
	real_t depth_falloff = is_pin() ? tip->get_pin()->depth_falloff : 1.0;
	for (int32_t chain_i = 0; chain_i < child_chains.size(); chain_i++) {
		Ref<IKBoneChain> chain = child_chains[chain_i];
		chain->update_pinned_list();
	}
	if (is_pin()) {
		effector_list.push_back(tip->get_pin());
	}
	if (!Math::is_zero_approx(depth_falloff)) {
		for (Ref<IKBoneChain> child : child_chains) {
			effector_list.append_array(child->effector_list);
		}
	}
	for (Ref<IKManipulator3D> effector : effector_list) {
		// TODO: 2021-05-02 fire Implement proper weights
		Vector<real_t> weights;
		weights.push_back(1.0f);
		{
			weights.push_back(1.0f);
			weights.push_back(1.0f);
		}
		{
			weights.push_back(1.0f);
			weights.push_back(1.0f);
		}
		{
			weights.push_back(1.0f);
			weights.push_back(1.0f);
		}
		heading_weights.append_array(weights);
		effector->create_headings(heading_weights);
	}
	int32_t n = heading_weights.size();
	target_headings.resize(n);
	tip_headings.resize(n);
}

void IKBoneChain::update_optimal_rotation(Ref<IKBone3D> p_for_bone, real_t p_damp, bool p_translate) {
	Vector<real_t> *weights = nullptr;
	PackedVector3Array heading_target = update_target_headings(p_for_bone, weights);
	PackedVector3Array heading_tip = update_tip_headings(p_for_bone);

	if (p_translate == true) {
		p_damp = Math_PI;
	}

	set_optimal_rotation(p_for_bone, heading_tip, heading_target, *weights, p_damp, p_translate);
}

Quaternion IKBoneChain::set_quadrance_angle(Quaternion p_quat, real_t p_cos_half_angle) const {
	double squared_sine = p_quat.get_axis().length_squared();
	Quaternion rot = p_quat;
	if (!Math::is_zero_approx(squared_sine)) {
		double inverse_coeff = Math::sqrt(((1.0f - (p_cos_half_angle * p_cos_half_angle)) / squared_sine));
		rot.x = inverse_coeff * p_quat.x;
		rot.y = inverse_coeff * p_quat.y;
		rot.z = inverse_coeff * p_quat.z;
		rot.w = p_quat.w < 0 ? -p_cos_half_angle : p_cos_half_angle;
	}
	return rot;
}

Quaternion IKBoneChain::clamp_to_angle(Quaternion p_quat, real_t p_angle) const {
	double cos_half_angle = Math::cos(0.5 * p_angle);
	return clamp_to_quadrance_angle(p_quat, cos_half_angle);
}

Quaternion IKBoneChain::clamp_to_quadrance_angle(Quaternion p_quat, real_t p_cos_half_angle) const {
	double new_coeff = 1.0 - (p_cos_half_angle * p_cos_half_angle);
	double current_coeff = p_quat.get_axis().length_squared();
	Quaternion rot = p_quat;
	if (new_coeff > current_coeff) {
		return rot;
	} else {
		double compositeCoeff = Math::sqrt(new_coeff / current_coeff);
		rot.x *= compositeCoeff;
		rot.y *= compositeCoeff;
		rot.z *= compositeCoeff;
		rot.w = p_quat.w < 0 ? -p_cos_half_angle : p_cos_half_angle;
	}
	return rot;
}

double IKBoneChain::set_optimal_rotation(Ref<IKBone3D> p_for_bone,
		PackedVector3Array &r_htip, PackedVector3Array &r_htarget, const Vector<real_t> &p_weights, float p_dampening, bool p_translate) {
	Quaternion rot;
	Vector3 translation;
	double sqrmsd = qcp.calc_optimal_rotation(r_htip, r_htarget, p_weights, rot, p_translate, translation);

	double bone_damp = p_for_bone->get_cos_half_dampen();

	if (!Math::is_equal_approx(p_dampening, -1.0f)) {
		bone_damp = p_dampening;
		rot = clamp_to_angle(rot, bone_damp);
	} else {
		rot = clamp_to_quadrance_angle(rot, bone_damp);
	}
	p_for_bone->set_global_pose_rot_delta(rot);
	p_for_bone->set_global_pose_translation_delta(translation);
	return sqrmsd;
}

void IKBoneChain::create_headings() {
}

PackedVector3Array IKBoneChain::update_target_headings(Ref<IKBone3D> p_for_bone, Vector<real_t> *&p_weights) {
	PackedVector3Array htarget = target_headings;
	p_weights = &heading_weights;
	int32_t index = 0; // Index is increased by effector->update_effector_target_headings() function
	for (int32_t effector_i = 0; effector_i < effector_list.size(); effector_i++) {
		Ref<IKManipulator3D> effector = effector_list[effector_i];
		effector->update_effector_target_headings(&htarget, index, p_for_bone, p_weights);
	}
	return htarget;
}

PackedVector3Array IKBoneChain::update_tip_headings(Ref<IKBone3D> p_for_bone) {
	PackedVector3Array heading_tip = tip_headings;
	int32_t index = 0; // Index is increased by effector->update_target_headings() function
	for (int32_t effector_i = 0; effector_i < effector_list.size(); effector_i++) {
		Ref<IKManipulator3D> effector = effector_list[effector_i];
		effector->update_effector_tip_headings(&heading_tip, index, p_for_bone);
	}
	return heading_tip;
}

void IKBoneChain::segment_solver(real_t p_damp, bool p_translate) {
	// TODO Make robust!

	for (Ref<IKBoneChain> child : child_chains) {
		child->segment_solver(p_damp, p_translate);
	}
	qcp_solver(p_damp, p_translate);
}

void IKBoneChain::qcp_solver(real_t p_damp, bool p_translate) {
	for (Ref<IKBone3D> current_bone : bones) {
		update_optimal_rotation(current_bone, p_damp, p_translate);
	}
}

void IKBoneChain::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_root_pinned"), &IKBoneChain::is_root_pinned);
	ClassDB::bind_method(D_METHOD("is_pin"), &IKBoneChain::is_pin);
}

IKBoneChain::IKBoneChain(Skeleton3D *p_skeleton, BoneId p_root_bone, const Ref<IKBoneChain> &p_parent) {
	skeleton = p_skeleton;
	root = Ref<IKBone3D>(memnew(IKBone3D(p_skeleton->get_bone_name(p_root_bone), p_skeleton, p_parent)));
	if (p_parent.is_valid()) {
		parent_chain = p_parent;
		root->set_parent(p_parent->get_tip());
	}
}