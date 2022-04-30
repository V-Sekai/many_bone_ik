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

Vector<Ref<IKBoneChain>> IKBoneChain::get_pinned_direct_descendents() const {
	return effector_direct_descendents;
}

BoneId IKBoneChain::find_root_bone_id(BoneId p_bone) {
	BoneId root_id = p_bone;
	while (skeleton->get_bone_parent(root_id) != -1) {
		root_id = skeleton->get_bone_parent(root_id);
	}

	return root_id;
}

void IKBoneChain::generate_skeleton_segments(const HashMap<BoneId, Ref<IKBone3D>> &p_map) {
	child_chains.clear();

	Ref<IKBone3D> temp_tip = root;
	while (true) {
		Vector<BoneId> children_with_pin_descendants = temp_tip->get_children_with_pin_descendants(skeleton, p_map);
		if (children_with_pin_descendants.size() > 1) {
			tip = temp_tip;
			for (int32_t child_i = 0; child_i < children_with_pin_descendants.size(); child_i++) {
				BoneId child_bone = children_with_pin_descendants[child_i];
				child_chains.push_back(Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, child_bone, p_map, tip))));
			}
			break;
		} else if (children_with_pin_descendants.size() == 1) {
			BoneId bone_id = children_with_pin_descendants[0];
			if (p_map.has(bone_id)) {
				Ref<IKBone3D> next = p_map[bone_id];
				next->set_parent(temp_tip);
				temp_tip = next;
			} else {
				Ref<IKBone3D> next = Ref<IKBone3D>(memnew(IKBone3D(bone_id, temp_tip)));
				temp_tip = next;
			}
		} else {
			tip = temp_tip;
			break;
		}
	}
	update_segmented_skeleton();
}

void IKBoneChain::update_segmented_skeleton() {
	update_effector_direct_descendents();
	generate_bones_map();
}

void IKBoneChain::update_effector_direct_descendents() {
	effector_direct_descendents.clear();
	if (is_pin()) {
		effector_direct_descendents.push_back(this);
	} else {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			Ref<IKBoneChain> child_segment = child_chains[child_i];
			effector_direct_descendents.append_array(child_segment->get_pinned_direct_descendents());
		}
	}
}

void IKBoneChain::generate_bones_map() {
	bones_map.clear();
	Ref<IKBone3D> current_bone = tip;
	Ref<IKBone3D> stop_on = root;
	while (current_bone.is_valid()) {
		bones_map[current_bone->get_bone_id()] = current_bone;
		if (current_bone == stop_on) {
			break;
		}
		current_bone = current_bone->get_parent();
	}
}

void IKBoneChain::generate_default_segments_from_root() {
	child_chains.clear();
	Ref<IKBone3D> temp_tip = root;
	while (true) {
		Vector<BoneId> children = skeleton->get_bone_children(temp_tip->get_bone_id());
		if (children.size() > 1) {
			tip = temp_tip;
			for (int32_t child_i = 0; child_i < children.size(); child_i++) {
				BoneId child_bone = children[child_i];
				Ref<IKBoneChain> child_segment = Ref<IKBoneChain>(memnew(IKBoneChain(skeleton, child_bone, bones_map, tip)));
				child_segment->generate_default_segments_from_root();
				child_chains.push_back(child_segment);
			}
			break;
		} else if (children.size() == 1) {
			BoneId bone_id = children[0];
			Ref<IKBone3D> next = Ref<IKBone3D>(memnew(IKBone3D(bone_id, temp_tip)));
			temp_tip = next;
		} else {
			tip = temp_tip;
			tip->create_pin();
			break;
		}
	}
	update_segmented_skeleton();
}

Ref<IKBoneChain> IKBoneChain::get_child_segment_containing(const Ref<IKBone3D> &p_bone) {
	if (bones_map.has(p_bone->get_bone_id())) {
		return this;
	} else {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			Ref<IKBoneChain> child_segment = child_chains.write[child_i]->get_child_segment_containing(p_bone);
			if (child_segment.is_valid()) {
				return child_segment;
			}
		}
	}
	return nullptr;
}

Ref<IKBone3D> IKBoneChain::find_bone(const BoneId p_bone_id) {
	if (bones_map.has(p_bone_id)) {
		return bones_map[p_bone_id];
	} else {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			Ref<IKBoneChain> child_segment = child_chains.write[child_i]->find_bone(p_bone_id);
			if (child_segment.is_valid()) {
				return child_segment->bones_map[p_bone_id];
			}
		}
	}
	return nullptr;
}

void IKBoneChain::get_bone_list(Vector<Ref<IKBone3D>> &p_list, bool p_recursive, bool p_debug_skeleton) const {
	if (p_recursive) {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			child_chains[child_i]->get_bone_list(p_list, p_recursive, p_debug_skeleton);
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
	list.reverse();
}

void IKBoneChain::update_pinned_list() {
	real_t depth_falloff = is_pin() ? tip->get_pin()->depth_falloff : 1.0;
	for (int32_t chain_i = 0; chain_i < child_chains.size(); chain_i++) {
		Ref<IKBoneChain> chain = child_chains[chain_i];
		chain->update_pinned_list();
		if (depth_falloff > CMP_EPSILON) {
			effector_list.append_array(chain->effector_list);
			for (int32_t w_i = 0; w_i < chain->heading_weights.size(); w_i++) {
				heading_weights.push_back(chain->heading_weights[w_i] * depth_falloff);
			}
		}
	}
	Vector<Ref<IKBone3D>> pins;
	get_bone_list(pins, true);
	for (int32_t pin_i = 0; pin_i < pins.size(); pin_i++) {
		Ref<IKPin3D> effector = pins[pin_i]->get_pin();
		if (effector.is_null()) {
			continue;
		}
		effector_list.push_back(effector);
		Vector<real_t> weights;
		weights.push_back(effector->weight * depth_falloff);
		{
			weights.push_back(effector->weight * depth_falloff);
			weights.push_back(effector->weight * depth_falloff);
		}
		{
			weights.push_back(effector->weight * depth_falloff);
			weights.push_back(effector->weight * depth_falloff);
		}
		{
			weights.push_back(effector->weight * depth_falloff);
			weights.push_back(effector->weight * depth_falloff);
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
	PackedVector3Array htarget = update_target_headings(weights);
	PackedVector3Array htip = update_tip_headings(p_for_bone);

	if (p_translate == true) {
		p_damp = Math_PI;
	}

	set_optimal_rotation(p_for_bone, htip, htarget, *weights, p_damp, p_translate);
}

Quaternion IKBoneChain::set_quadrance_angle(Quaternion p_quat, real_t p_cos_half_angle) const {
	float squared_sine = p_quat.x * p_quat.x + p_quat.y * p_quat.y + p_quat.z * p_quat.z;
	Quaternion rot = p_quat;
	if (!Math::is_zero_approx(squared_sine)) {
		float inverse_coeff = Math::sqrt(((1.0f - (p_cos_half_angle * p_cos_half_angle)) / squared_sine));
		rot.x = inverse_coeff * p_quat.x;
		rot.y = inverse_coeff * p_quat.y;
		rot.z = inverse_coeff * p_quat.z;
		rot.w = p_quat.w < 0 ? -p_cos_half_angle : p_cos_half_angle;
	}
	return rot;
}

Quaternion IKBoneChain::clamp_to_angle(Quaternion p_quat, real_t p_angle) const {
	float cos_half_angle = Math::cos(0.5f * p_angle);
	return clamp_to_quadrance_angle(p_quat, cos_half_angle);
}

Quaternion IKBoneChain::clamp_to_quadrance_angle(Quaternion p_quat, real_t p_cos_half_angle) const {
	float new_coeff = 1.0f - (p_cos_half_angle * p_cos_half_angle);
	float current_coeff = p_quat.x * p_quat.x + p_quat.y * p_quat.y + p_quat.z * p_quat.z;
	Quaternion rot = p_quat;
	if (new_coeff > current_coeff) {
		return rot;
	} else {
		float compositeCoeff = Math::sqrt(new_coeff / current_coeff);
		rot.x *= compositeCoeff;
		rot.y *= compositeCoeff;
		rot.z *= compositeCoeff;
		rot.w = p_quat.w < 0 ? -p_cos_half_angle : p_cos_half_angle;
	}
	return rot;
}

real_t IKBoneChain::set_optimal_rotation(Ref<IKBone3D> p_for_bone,
		PackedVector3Array &r_htip, PackedVector3Array &r_htarget, const Vector<real_t> &p_weights, float p_dampening, bool p_translate) {
	Quaternion rot;
	Vector3 translation;
	real_t sqrmsd = qcp.calc_optimal_rotation(r_htip, r_htarget, p_weights, rot, p_translate, translation);

	float bone_damp = p_for_bone->get_cos_half_dampen();

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

PackedVector3Array IKBoneChain::update_target_headings(Vector<real_t> *&p_weights) {
	PackedVector3Array htarget = target_headings;
	p_weights = &heading_weights;
	int32_t index = 0; // Index is increased by effector->update_effector_target_headings() function
	for (int32_t effector_i = 0; effector_i < effector_list.size(); effector_i++) {
		Ref<IKPin3D> effector = effector_list[effector_i];
		effector->update_effector_target_headings(&htarget, index, p_weights);
	}
	return htarget;
}

PackedVector3Array IKBoneChain::update_tip_headings(Ref<IKBone3D> p_for_bone) {
	PackedVector3Array htip = tip_headings;
	int32_t index = 0; // Index is increased by effector->update_target_headings() function
	for (int32_t effector_i = 0; effector_i < effector_list.size(); effector_i++) {
		Ref<IKPin3D> effector = effector_list[effector_i];
		effector->update_effector_tip_headings(p_for_bone, &htip, index);
	}
	return htip;
}

void IKBoneChain::segment_solver(real_t p_damp, bool p_translate) {
	if (child_chains.size() == 0 && !is_pin()) {
		return;
	} else if (!is_pin()) {
		for (int32_t child_i = 0; child_i < child_chains.size(); child_i++) {
			Ref<IKBoneChain> child = child_chains[child_i];
			child->segment_solver(p_damp, p_translate);
		}
	}
	qcp_solver(p_damp, p_translate);
}

void IKBoneChain::qcp_solver(real_t p_damp, bool p_translate) {
	Vector<Ref<IKBone3D>> list;
	get_bone_list(list, false);
	for (int32_t bone_i = 0; bone_i < list.size(); bone_i++) {
		Ref<IKBone3D> current_bone = list[bone_i];

		update_optimal_rotation(current_bone, p_damp, p_translate);

		if (current_bone == root) {
			break;
		}
	}
}

void IKBoneChain::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_root_pinned"), &IKBoneChain::is_root_pinned);
	ClassDB::bind_method(D_METHOD("is_pin"), &IKBoneChain::is_pin);
}

IKBoneChain::IKBoneChain(Skeleton3D *p_skeleton, BoneId p_root_bone,
		const HashMap<BoneId, Ref<IKBone3D>> &p_map, const Ref<IKBoneChain> &p_parent) {
	skeleton = p_skeleton;
	if (p_map.has(p_root_bone)) {
		root = p_map[p_root_bone];
	} else {
		root = Ref<IKBone3D>(memnew(IKBone3D(p_root_bone)));
	}
	if (p_parent.is_valid()) {
		parent_chain = p_parent;
		root->set_parent(p_parent->get_tip());
	}
	generate_skeleton_segments(p_map);
}