/*************************************************************************/
/*  skeleton_modification_3d_ewbik.cpp                                   */
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

#include "skeleton_modification_3d_ewbik.h"
#include "core/templates/map.h"

int32_t SkeletonModification3DEWBIK::get_ik_iterations() const {
	return ik_iterations;
}

void SkeletonModification3DEWBIK::set_ik_iterations(int32_t p_iterations) {
	ERR_FAIL_COND_MSG(p_iterations <= 0, "EWBIK max iterations must be at least one. Set enabled to false to disable the EWBIK simulation.");
	ik_iterations = p_iterations;
}

String SkeletonModification3DEWBIK::get_root_bone() const {
	return root_bone;
}

void SkeletonModification3DEWBIK::set_root_bone(String p_root_bone) {
	root_bone = p_root_bone;
	if (skeleton) {
		root_bone_index = skeleton->find_bone(root_bone);
		update_segments();
	}
}

BoneId SkeletonModification3DEWBIK::get_root_bone_index() const {
	return root_bone_index;
}

void SkeletonModification3DEWBIK::set_root_bone_index(BoneId p_index) {
	root_bone_index = p_index;
	if (skeleton) {
		root_bone = skeleton->get_bone_name(p_index);
		update_segments();
	}
}

void SkeletonModification3DEWBIK::set_effector_count(int32_t p_value) {
	multi_effector.resize(p_value);
	for (int32_t i = effector_count; i < p_value; i++) {
		Ref<EWBIKShadowBone3D> bone = Ref<EWBIKShadowBone3D>(memnew(EWBIKShadowBone3D()));
		bone->create_effector();
		multi_effector.write[i] = bone;
	}
	effector_count = p_value;
	if (skeleton) {
		update_segments();
	}

	notify_property_list_changed();
}

int32_t SkeletonModification3DEWBIK::get_effector_count() const {
	return effector_count;
}

void SkeletonModification3DEWBIK::add_effector(String p_name, NodePath p_target_node, bool p_use_node_xform, Transform p_target_xform) {
	Ref<EWBIKShadowBone3D> effector_bone = Ref<EWBIKShadowBone3D>(memnew(EWBIKShadowBone3D(skeleton, p_name)));
	Ref<EWBIKBoneEffector3D> effector;
	effector.instance();
	effector->set_target_node(p_target_node);
	effector->set_use_target_node_transform(p_use_node_xform);
	effector->set_target_transform(p_target_xform);
	effector_bone->set_effector(effector);
	multi_effector.push_back(effector_bone);
	effector_count++;

	if (skeleton) {
		update_segments();
	}
}

Ref<EWBIKShadowBone3D> SkeletonModification3DEWBIK::get_effector(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_effector.size(), NULL);
	Ref<EWBIKShadowBone3D> effector = multi_effector[p_index];
	return effector;
}

void SkeletonModification3DEWBIK::set_effector(int32_t p_index, Ref<EWBIKShadowBone3D> p_effector) {
	ERR_FAIL_COND(p_effector.is_null());
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.write[p_index] = p_effector;

	if (skeleton) {
		update_segments();
	}
}

Vector<Ref<EWBIKShadowBone3D>> SkeletonModification3DEWBIK::get_bone_effectors() const {
	return multi_effector;
}

int32_t SkeletonModification3DEWBIK::find_effector(String p_name) {
	for (int32_t effector_i = 0; effector_i < multi_effector.size(); effector_i++) {
		if (multi_effector[effector_i].is_valid() && multi_effector[effector_i]->get_name() == p_name) {
			return effector_i;
		}
	}
	return -1;
}

void SkeletonModification3DEWBIK::remove_effector(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.remove(p_index);
	effector_count--;

	if (skeleton) {
		update_segments();
	}
}

void SkeletonModification3DEWBIK::execute(float delta) {
	ERR_FAIL_COND_MSG(!stack || !is_setup || skeleton == nullptr,
			"Modification is not setup and therefore cannot execute!");
	if (!enabled)
		return;

	solve(stack->get_strength());
	execution_error_found = false;
}

void SkeletonModification3DEWBIK::setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;
	if (!stack) {
		return;
	}
	skeleton = stack->skeleton;
	if (!skeleton) {
		return;
	}

	if (root_bone.is_empty()) {
		Vector<int32_t> roots;
		for (int32_t bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
			int32_t parent = skeleton->get_bone_parent(bone_i);
			if (parent == -1) {
				roots.push_back(bone_i);
			}
		}
		if (roots.size()) {
			set_root_bone_index(roots[0]);
		}
	} else {
		if (root_bone_index == -1)
			set_root_bone(root_bone);
	}
	ERR_FAIL_COND(root_bone.is_empty());

	if (!effector_count) {
		segmented_skeleton = Ref<EWBIKSegmentedSkeleton3D>(memnew(EWBIKSegmentedSkeleton3D(skeleton, root_bone_index)));
		segmented_skeleton->generate_default_segments_from_root();
		Vector<Ref<EWBIKSegmentedSkeleton3D>> effector_chains = segmented_skeleton->get_effector_descendents();
		effector_count = effector_chains.size();
		multi_effector.resize(effector_count);
		for (int32_t chain_i = 0; chain_i < effector_count; chain_i++) {
			multi_effector.write[chain_i] = effector_chains[chain_i]->get_tip();
			Ref<EWBIKSegmentedSkeleton3D> segment = effector_chains[chain_i];
		}
		update_effectors_map();
	}

	is_setup = true;
	execution_error_found = false;
	notify_property_list_changed();
}

void SkeletonModification3DEWBIK::solve(float blending_delta) {
	if (blending_delta <= 0.01f) {
		return; // Skip solving
	}

	if (effector_count && segmented_skeleton.is_valid() && segmented_skeleton->get_effector_descendents_size() > 0)
		iterated_improved_solver();
}

void SkeletonModification3DEWBIK::iterated_improved_solver() {
	segmented_skeleton->align_axes_to_bones();
	for (int i = 0; i < ik_iterations; i++) {
	}
	// Ref<EWBIKSegmentedSkeleton3D> armature = p_start_from;
	// if (armature.is_null()) {
	// 	return;
	// }
	// Ref<EWBIKSegmentedSkeleton3D> pinned_root_chain = armature;
	// if (pinned_root_chain.is_null() && p_root_bone != -1) {
	// 	armature = armature->chain_root->find_child(p_root_bone);
	// } else {
	// 	armature = pinned_root_chain;
	// }
	// if (armature.is_valid() && armature->targets.size() > 0) {
	// 	armature->align_axes_to_bones();
	// 	if (p_iterations == -1) {
	// 		p_iterations = armature->ik_iterations;
	// 	}
	// 	float totalIterations = p_iterations;
	// 	if (p_stabilization_passes == -1) {
	// 		p_stabilization_passes = armature->stabilization_passes;
	// 	}
	// 	for (int i = 0; i < p_iterations; i++) {
	// 		if (!armature->base_bone->is_bone_effector(armature->base_bone) && armature->get_child_chains().size()) {
	// 			update_optimal_rotation_to_target_descendants(armature->skeleton, armature->chain_root, Math_PI, true, armature->localized_target_headings, armature->localized_effector_headings, armature->weights, p_qcp, i, totalIterations);
	// 			armature->set_processed(false);
	// 			Vector<Ref<EWBIKSegmentedSkeleton3D>> segmented_armature = armature->get_child_chains();
	// 			for (int32_t i = 0; i < segmented_armature.size(); i++) {
	// 				grouped_recursive_chain_solver(segmented_armature[i], armature->dampening, armature->stabilization_passes, i, totalIterations);
	// 			}
	// 		} else {
	// 			grouped_recursive_chain_solver(armature, p_dampening, p_stabilization_passes, i, totalIterations);
	// 		}
	// 	}
	// 	armature->recursively_align_bones_to_sim_axes_from(armature->chain_root);
	// }
}

void SkeletonModification3DEWBIK::update_segments() {
	if (effector_count) {
		update_effectors_map();
		segmented_skeleton = Ref<EWBIKSegmentedSkeleton3D>(memnew(EWBIKSegmentedSkeleton3D(skeleton, root_bone_index, effectors_map)));
	}
}

void SkeletonModification3DEWBIK::update_effectors_map() {
	effectors_map.clear();
	for (int32_t index = 0; index < effector_count; index++) {
		Ref<EWBIKShadowBone3D> effector_bone = multi_effector[index];
		effectors_map[effector_bone->get_bone_id()] = effector_bone;
	}
}

void SkeletonModification3DEWBIK::_validate_property(PropertyInfo &property) const {
	if (property.name == "root_bone") {
		if (skeleton) {
			String names;
			for (int i = 0; i < skeleton->get_bone_count(); i++) {
				if (i > 0) {
					names += ",";
				}
				names += skeleton->get_bone_name(i);
			}

			property.hint = PROPERTY_HINT_ENUM;
			property.hint_string = names;
		} else {
			property.hint = PROPERTY_HINT_NONE;
			property.hint_string = "";
		}
	}
}

void SkeletonModification3DEWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "ik_iterations", PROPERTY_HINT_RANGE, "0,65535,1"));
	p_list->push_back(PropertyInfo(Variant::INT, "effector_count", PROPERTY_HINT_RANGE, "0,65535,1"));
	for (int i = 0; i < effector_count; i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, "effectors/" + itos(i) + "/name"));
		p_list->push_back(
				PropertyInfo(Variant::NODE_PATH, "effectors/" + itos(i) + "/target_node"));
		p_list->push_back(
				PropertyInfo(Variant::BOOL, "effectors/" + itos(i) + "/use_node_transform"));
		p_list->push_back(
				PropertyInfo(Variant::TRANSFORM, "effectors/" + itos(i) + "/target_transform"));
	}
}

bool SkeletonModification3DEWBIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "ik_iterations") {
		r_ret = get_ik_iterations();
		return true;
	} else if (name == "effector_count") {
		r_ret = get_effector_count();
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, get_effector_count(), false);
		ERR_FAIL_COND_V(get_effector(index).is_null(), false);
		if (what == "name") {
			r_ret = get_effector(index)->get_name();
			return true;
		} else if (what == "target_node") {
			r_ret = get_effector(index)->get_effector()->get_target_node();
			return true;
		} else if (what == "use_node_transform") {
			r_ret = get_effector(index)->get_effector()->get_use_target_node_transform();
			return true;
		} else if (what == "target_transform") {
			r_ret = get_effector(index)->get_effector()->get_target_transform();
			return true;
		}
	}

	return false;
}

bool SkeletonModification3DEWBIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "ik_iterations") {
		set_ik_iterations(p_value);
		return true;
	} else if (name == "effector_count") {
		set_effector_count(p_value);
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, effector_count, false);
		Ref<EWBIKShadowBone3D> effector = get_effector(index);
		if (effector.is_null()) {
			effector.instance();
			effector->create_effector();
			set_effector(index, effector);
		}
		if (what == "name") {
			name = p_value;
			ERR_FAIL_COND_V(name.is_empty(), false);
			effector->set_name(name);

			return true;
		} else if (what == "target_node") {
			effector->get_effector()->set_target_node(p_value);

			return true;
		} else if (what == "use_node_transform") {
			effector->get_effector()->set_use_target_node_transform(p_value);

			return true;
		} else if (what == "target_transform") {
			effector->get_effector()->set_target_transform(p_value);

			return true;
		}
	}

	return false;
}

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ik_iterations"), &SkeletonModification3DEWBIK::get_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_ik_iterations", "iterations"), &SkeletonModification3DEWBIK::set_ik_iterations);
	ClassDB::bind_method(D_METHOD("set_root_bone", "root_bone"), &SkeletonModification3DEWBIK::set_root_bone);
	ClassDB::bind_method(D_METHOD("get_root_bone"), &SkeletonModification3DEWBIK::get_root_bone);
	ClassDB::bind_method(D_METHOD("get_effector_count"), &SkeletonModification3DEWBIK::get_effector_count);
	ClassDB::bind_method(D_METHOD("set_effector_count", "count"),
			&SkeletonModification3DEWBIK::set_effector_count);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node", "target_transform", "budget"), &SkeletonModification3DEWBIK::add_effector);
	ClassDB::bind_method(D_METHOD("get_effector", "index"), &SkeletonModification3DEWBIK::get_effector);
	ClassDB::bind_method(D_METHOD("set_effector", "index", "effector"), &SkeletonModification3DEWBIK::set_effector);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "root_bone"), "set_root_bone", "get_root_bone");
}

SkeletonModification3DEWBIK::SkeletonModification3DEWBIK() {
	enabled = true;
}

SkeletonModification3DEWBIK::~SkeletonModification3DEWBIK() {
}
