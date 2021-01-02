/*************************************************************************/
/*  skeleton_ik_constraints.cpp                                          */
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
#include "bone_effector.h"
#include "direction_constraint.h"
#include "kusudama_constraint.h"
#include "scene/3d/skeleton_3d.h"
#include "shadow_skeleton_bone.h"

void SkeletonModification3DEWBIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_effector_count"), &SkeletonModification3DEWBIK::get_effector_count);
	ClassDB::bind_method(D_METHOD("set_effector_count", "count"),
			&SkeletonModification3DEWBIK::set_effector_count);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node", "target_transform", "budget"), &SkeletonModification3DEWBIK::add_effector);
	ClassDB::bind_method(D_METHOD("get_effector", "index"), &SkeletonModification3DEWBIK::get_effector);
	ClassDB::bind_method(D_METHOD("set_effector", "index", "effector"), &SkeletonModification3DEWBIK::set_effector);
	ClassDB::bind_method(D_METHOD("set_skeleton_ik_data", "skeleton_ik_data"), &SkeletonModification3DEWBIK::set_state);
	ClassDB::bind_method(D_METHOD("get_skeleton_ik_data"), &SkeletonModification3DEWBIK::get_state);
}

void SkeletonModification3DEWBIK::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "effector_count", PROPERTY_HINT_RANGE, "0,65535,1"));
	for (int i = 0; i < effector_count; i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, "effectors/" + itos(i) + "/name"));
		p_list->push_back(
				PropertyInfo(Variant::TRANSFORM, "effectors/" + itos(i) + "/target_transform"));
		p_list->push_back(
				PropertyInfo(Variant::NODE_PATH, "effectors/" + itos(i) + "/target_node"));
		p_list->push_back(
				PropertyInfo(Variant::FLOAT, "effectors/" + itos(i) + "/budget", PROPERTY_HINT_RANGE, "0,16,or_greater"));
	}
	p_list->push_back(
			PropertyInfo(Variant::STRING, "root_bone"));
	p_list->push_back(
			PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_RESOURCE_TYPE, "EWBIKSkeletonIKState"));
}

bool SkeletonModification3DEWBIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "root_bone") {
		r_ret = get_root_bone();
		return true;
	} else if (name == "state") {
		r_ret = get_state();
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
		} else if (what == "target_transform") {
			r_ret = get_effector(index)->get_target_transform();
			return true;
		} else if (what == "target_node") {
			r_ret = get_effector(index)->get_target_node();
			return true;
		} else if (what == "budget") {
			r_ret = get_effector(index)->get_budget_ms();
			return true;
		}
	}
	return false;
}

bool SkeletonModification3DEWBIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;

	if (name == "root_bone") {
		set_root_bone(p_value);
		return true;
	} else if (name == "state") {
		set_state(p_value);
		return true;
	} else if (name == "effector_count") {
		set_effector_count(p_value);
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, effector_count, false);
		Ref<EWBIKBoneEffector> effector = get_effector(index);
		if (effector.is_null()) {
			effector.instance();
			set_effector(index, effector);
		}
		if (what == "name") {
			name = p_value;
			ERR_FAIL_COND_V(name.is_empty(), false);
			effector->set_name(name);
			_change_notify();
			return true;
		} else if (what == "target_node") {
			effector->set_target_node(p_value);
			_change_notify();
			return true;
		} else if (what == "target_transform") {
			effector->set_target_transform(p_value);
			_change_notify();
			return true;
		} else if (what == "budget") {
			effector->set_budget_ms(p_value);
			_change_notify();
			return true;
		}
	}
	return false;
}

String SkeletonModification3DEWBIK::get_root_bone() const {
	return root_bone;
}

void SkeletonModification3DEWBIK::set_root_bone(String p_root_bone) {
	root_bone = p_root_bone;
}

SkeletonModification3DEWBIK::SkeletonModification3DEWBIK() {
	enabled = true;
	qcp_convergence_check.instance();
	qcp_convergence_check->set_precision(FLT_EPSILON, FLT_EPSILON);
}

SkeletonModification3DEWBIK::~SkeletonModification3DEWBIK() {
}

void SkeletonModification3DEWBIK::set_effector_count(int32_t p_value) {
	multi_effector.resize(p_value);
	effector_count = p_value;
	_change_notify();
}

int32_t SkeletonModification3DEWBIK::get_effector_count() const {
	return effector_count;
}

Ref<EWBIKBoneEffector> SkeletonModification3DEWBIK::get_effector(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_effector.size(), NULL);
	Ref<EWBIKBoneEffector> effector = multi_effector[p_index];
	return effector;
}

void SkeletonModification3DEWBIK::set_effector(int32_t p_index, Ref<EWBIKBoneEffector> p_effector) {
	ERR_FAIL_COND(p_effector.is_null());
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.write[p_index] = p_effector;
	_change_notify();
}

Vector<Ref<EWBIKBoneEffector>> SkeletonModification3DEWBIK::get_bone_effectors() const {
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
	_change_notify();
}

void SkeletonModification3DEWBIK::execute(float delta) {
	ERR_FAIL_COND_MSG(!stack || !is_setup || stack->skeleton == nullptr,
			"Modification is not setup and therefore cannot execute!");
	if (!enabled) {
		return;
	}
	if (task.is_valid()) {
		solve(task, stack->get_strength());
	}
}

void SkeletonModification3DEWBIK::setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;
	if (!stack) {
		return;
	}
	Skeleton3D *skeleton = stack->skeleton;
	if (skeleton) {
		if (!constraint_count) {
			Vector<int32_t> roots;
			for (int32_t bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
				int32_t parent = skeleton->get_bone_parent(bone_i);
				if (parent == -1) {
					roots.push_back(bone_i);
				}
			}
			if (roots.size()) {
				String root_name = skeleton->get_bone_name(roots[0]);
				root_bone = root_name;
			}
		}
		ERR_FAIL_COND(root_bone.is_empty());
		if (!constraint_count) {
			BoneId _bone = skeleton->find_bone(root_bone);
			Ref<EWBIKSegmentedSkeleton3D> chain_item;
			chain_item.instance();
			chain_item->bone = _bone;
			Ref<EWBIKSegmentedSkeleton3D> bone_chain;
			bone_chain.instance();
			Ref<SkeletonModification3DEWBIK> mod = this;
			bone_chain->init(skeleton, mod, multi_effector, bone_chain, nullptr, chain_item);
			Vector<StringName> effectors = bone_chain->get_default_effectors(skeleton, bone_chain, bone_chain);
			set_effector_count(0);
			for (int32_t effector_i = 0; effector_i < effectors.size(); effector_i++) {
				add_effector(effectors[effector_i]);
			}
			register_constraint(skeleton);
		}
		skeleton_ik_state.instance();
		skeleton_ik_state->init(this);
		task = create_simple_task(skeleton, root_bone, -1.0f, 10.0f, this);
	}
	is_setup = true;
	execution_error_found = false;
}

void SkeletonModification3DEWBIK::iterated_improved_solver(Ref<QCP> p_qcp, int32_t p_root_bone, Ref<EWBIKSegmentedSkeleton3D> start_from, float dampening, int iterations, int p_stabilization_passes) {
	Ref<EWBIKSegmentedSkeleton3D> armature = start_from;
	if (armature.is_null()) {
		return;
	}
	Ref<EWBIKSegmentedSkeleton3D> pinned_root_chain = armature;
	if (pinned_root_chain.is_null() && p_root_bone != -1) {
		armature = armature->chain_root->find_child(p_root_bone);
	} else {
		armature = pinned_root_chain;
	}
	if (armature.is_valid() && armature->get_bones().size() > 0) {
		armature->align_axes_to_bones();
		if (iterations == -1) {
			iterations = armature->ik_iterations;
		}
		float totalIterations = iterations;
		if (p_stabilization_passes == -1) {
			p_stabilization_passes = armature->stabilization_passes;
		}
		for (int i = 0; i < iterations; i++) {
			if (!armature->base_bone->is_bone_effector(armature->base_bone) && armature->get_child_chains().size()) {
				update_optimal_rotation_to_target_descendants(armature->skeleton, armature, armature->dampening, true, armature->localized_target_headings, armature->localized_effector_headings, armature->weights, p_qcp, i, totalIterations);
				armature->set_processed(false);
				Vector<Ref<EWBIKSegmentedSkeleton3D>> segmented_armature = armature->get_child_chains();
				for (int32_t i = 0; i < segmented_armature.size(); i++) {
					grouped_recursive_chain_solver(segmented_armature[i], armature->dampening, armature->stabilization_passes, i, totalIterations);
				}
			} else {
				grouped_recursive_chain_solver(armature, dampening, p_stabilization_passes, i, totalIterations);
			}
		}
		// TODO
		// armature.recursivelyAlignBonesToSimAxesFrom(armature.segmentRoot);
		// TODO
		// recursivelyNotifyBonesOfCompletedIKSolution(armature);
	}
}

void SkeletonModification3DEWBIK::grouped_recursive_chain_solver(Ref<EWBIKSegmentedSkeleton3D> p_start_from, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations) {
	recursive_chain_solver(p_start_from, p_dampening, p_stabilization_passes, p_iteration, p_total_iterations);
	Vector<Ref<EWBIKSegmentedSkeleton3D>> chains = p_start_from->get_child_chains();
	for (int32_t i = 0; i < chains.size(); i++) {
		grouped_recursive_chain_solver(chains[i], p_dampening, p_stabilization_passes, p_iteration, p_total_iterations);
	}
}

void SkeletonModification3DEWBIK::recursive_chain_solver(Ref<EWBIKSegmentedSkeleton3D> p_armature, float p_dampening, int p_stabilization_passes, int p_iteration, float p_total_iterations) {
	if (!p_armature->get_child_chains().size() && p_armature->is_bone_effector(p_armature)) {
		return;
	} else if (!p_armature->is_bone_effector(p_armature)) {
		Vector<Ref<EWBIKSegmentedSkeleton3D>> chains = p_armature->get_child_chains();
		for (int32_t i = 0; i < chains.size(); i++) {
			recursive_chain_solver(chains[i], p_dampening, p_stabilization_passes, p_iteration, p_total_iterations);
			chains.write[i]->set_processed(true);
		}
	}
	QCPSolver(
			p_armature->skeleton,
			p_armature,
			p_armature->dampening,
			false,
			p_iteration,
			p_stabilization_passes,
			p_total_iterations);
}

void SkeletonModification3DEWBIK::apply_bone_chains(float p_strength, Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_current_chain) {
	ERR_FAIL_COND(!p_current_chain->is_chain_active());
	Ref<EWBIKState> state = p_current_chain->mod->get_state();
	{
		Transform shadow_pose = state->get_shadow_pose_global(p_current_chain->bone);
		p_skeleton->set_bone_global_pose_override(p_current_chain->bone, shadow_pose, p_strength, true);
		p_skeleton->force_update_bone_children_transforms(p_current_chain->bone);
		Ref<KusudamaConstraint> kusudama = state->get_constraint(p_current_chain->bone);
		Transform shadow_constraint = state->get_shadow_constraint_pose_global(p_current_chain->bone);
		kusudama->set_constraint_axes(shadow_constraint);
	}
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bones = p_current_chain->get_bones();
	for (int32_t bone_i = 0; bone_i < bones.size(); bone_i++) {
		Transform shadow_pose = state->get_shadow_pose_global(bone_i);
		p_skeleton->set_bone_global_pose_override(bone_i, shadow_pose, p_strength, true);
		p_skeleton->force_update_bone_children_transforms(bone_i);
		Ref<KusudamaConstraint> kusudama = state->get_constraint(bone_i);
		Transform shadow_constraint = state->get_shadow_constraint_pose_global(bone_i);
		kusudama->set_constraint_axes(shadow_constraint);
	}
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bone_chains = p_current_chain->get_child_chains();
	for (int32_t i = 0; i < bone_chains.size(); i++) {
		apply_bone_chains(p_strength, p_skeleton, bone_chains[i]);
	}
}

void SkeletonModification3DEWBIK::add_effector(String p_name, NodePath p_node, Transform p_transform, real_t p_budget) {
	Ref<EWBIKBoneEffector> effector;
	effector.instance();
	effector->set_name(p_name);
	effector->set_target_node(p_node);
	effector->set_target_transform(p_transform);
	effector->set_budget_ms(p_budget);
	multi_effector.push_back(effector);
	effector_count++;
	_change_notify();
}

void SkeletonModification3DEWBIK::register_constraint(Skeleton3D *p_skeleton) {
	ERR_FAIL_COND(!p_skeleton);
	for (int32_t bone_i = 0; bone_i < p_skeleton->get_bone_count(); bone_i++) {
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		String bone_name = p_skeleton->get_bone_name(bone_i);
		constraint->set_name(bone_name);
		skeleton_ik_state->set_constraint(bone_i, constraint);
		constraint_count++;
	}
	_change_notify();
}

void SkeletonModification3DEWBIK::QCPSolver(
		Skeleton3D *p_skeleton,
		Ref<EWBIKSegmentedSkeleton3D> p_chain,
		float p_dampening,
		bool p_inverse_weighting,
		int p_stabilization_passes,
		int p_iteration,
		float p_total_iterations) {
	for (int32_t tip_i = 0; tip_i < p_chain->targets.size(); tip_i++) {
		if (p_chain->targets[tip_i].is_null()) {
			continue;
		}
		Ref<EWBIKSegmentedSkeleton3D> start_from = p_chain->targets[tip_i]->chain_item;
		Ref<EWBIKSegmentedSkeleton3D> stop_after = p_chain->chain_root;

		Ref<EWBIKSegmentedSkeleton3D> current_bone = start_from;
		//if the tip is pinned, it should have already been oriented before this function was called.
		while (current_bone.is_valid()) {
			if (!current_bone->ik_orientation_lock) {
				update_optimal_rotation_to_target_descendants(p_skeleton, p_chain, current_bone, p_dampening, false,
						p_stabilization_passes, p_iteration, p_total_iterations);
			}
			if (current_bone == stop_after) {
				current_bone = Ref<EWBIKSegmentedSkeleton3D>(nullptr);
			} else {
				current_bone = current_bone->parent_item;
			}
		}
	}
}

Ref<EWBIKSegmentedSkeleton3D> EWBIKSegmentedSkeleton3D::find_child(const int p_bone_id) {
	return chain_root->bone_segment_map[p_bone_id];
}

Ref<EWBIKSegmentedSkeleton3D> EWBIKSegmentedSkeleton3D::add_child(const int p_bone_id) {
	const int infant_child_id = children.size();
	children.resize(infant_child_id + 1);
	if (children.write[infant_child_id].is_null()) {
		children.write[infant_child_id].instance();
	}
	children.write[infant_child_id]->bone = p_bone_id;
	children.write[infant_child_id]->parent_item = Ref<EWBIKSegmentedSkeleton3D>(this);
	return children.write[infant_child_id];
}

void EWBIKSegmentedSkeleton3D::set_axes_to_returned(Transform p_global, Transform p_to_set, Transform p_limiting_axes,
		float p_cos_half_angle_dampen, float p_angle_dampen) {
	Ref<KusudamaConstraint> constraint = mod->get_state()->get_constraint(bone);
	if (constraint.is_valid()) {
		constraint->set_axes_to_returnful(p_global, p_to_set, p_limiting_axes, p_cos_half_angle_dampen,
				p_angle_dampen);
	}
}

void EWBIKSegmentedSkeleton3D::set_axes_to_be_snapped(Transform p_to_set, Transform p_limiting_axes,
		float p_cos_half_angle_dampen) {
	Ref<KusudamaConstraint> constraint = mod->get_state()->get_constraint(bone);
	if (constraint.is_valid()) {
		constraint->set_axes_to_snapped(p_to_set, p_limiting_axes, p_cos_half_angle_dampen);
	}
}

bool SkeletonModification3DEWBIK::build_chain(Ref<EWBIKTask> p_task) {
	ERR_FAIL_COND_V(-1 == p_task->root_bone, false);
	Ref<EWBIKSegmentedSkeleton3D> chain = p_task->chain;
	chain->chain_root = chain;
	chain->mod = p_task->ewbik;
	chain->bone = p_task->root_bone;
	chain->init(p_task->skeleton, p_task->ewbik, p_task->ewbik->multi_effector, chain, nullptr, chain);
	chain->filter_and_merge_child_chains();
	chain->bone = p_task->root_bone;
	chain->pb = p_task->skeleton->get_physical_bone(chain->bone);
	return true;
}

void SkeletonModification3DEWBIK::solve_simple(Ref<EWBIKTask> p_task) {
	iterated_improved_solver(p_task->qcp, p_task->root_bone, p_task->chain, p_task->dampening, p_task->max_iterations, p_task->stabilizing_passes);
}

Ref<EWBIKTask> SkeletonModification3DEWBIK::create_simple_task(Skeleton3D *p_sk, String p_root_bone,
		float p_dampening, int p_stabilizing_passes,
		Ref<SkeletonModification3DEWBIK> p_constraints) {
	Ref<EWBIKTask> task;
	task.instance();
	task->qcp = p_constraints->qcp_convergence_check;
	task->skeleton = p_sk;
	BoneId bone = p_sk->find_bone(p_root_bone);
	task->root_bone = bone;
	Ref<EWBIKSegmentedSkeleton3D> bone_item;
	bone_item.instance();
	bone_item->bone = bone;
	ERR_FAIL_COND_V(task->root_bone == -1, NULL);
	ERR_FAIL_COND_V(p_constraints.is_null(), NULL);
	{
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		constraint->set_name(p_sk->get_bone_name(task->root_bone));
		p_constraints->skeleton_ik_state->set_constraint(0, constraint);
	}
	task->dampening = p_dampening;
	task->stabilizing_passes = p_stabilizing_passes;
	ERR_FAIL_COND_V(!p_constraints->multi_effector.size(), nullptr);
	task->ewbik = p_constraints;
	if (!build_chain(task)) {
		return NULL;
	}
	task->chain->chain_root->print_bone_chains(task->skeleton, task->chain->chain_root);
	return task;
}

void SkeletonModification3DEWBIK::solve(Ref<EWBIKTask> p_task, float blending_delta) {
	if (blending_delta <= 0.01f) {
		return; // Skip solving
	}

	for (int32_t bone_i = 0; bone_i < p_task->skeleton->get_bone_count(); bone_i++) {
		Skeleton3D *skeleton = p_task->skeleton;
		Transform xform = skeleton->get_bone_pose(bone_i) * skeleton->get_bone_custom_pose(bone_i);
		p_task->ewbik->skeleton_ik_state->set_shadow_bone_pose_local(bone_i, xform);
	}

	// for (int32_t constraint_i = 0; constraint_i < p_task->dmik->get_constraint_count(); constraint_i++) {
	// 	Ref<KusudamaConstraint> constraint = p_task->dmik->get_constraint(constraint_i);
	// 	ERR_CONTINUE(constraint.is_null());
	// 	for (int32_t direction_i = 0; direction_i < constraint->get_direction_count(); direction_i++) {
	// 		Ref<DirectionConstraint> direction = constraint->get_direction(direction_i);
	// 		if (direction.is_null()) {
	// 			continue;
	// 		}
	// 		Vector3 cp = direction->get_control_point();
	// 		direction->set_control_point(cp.normalized());
	// 		constraint->set_direction(direction_i, direction);
	// 	}
	// }
	int effector_count = p_task->ewbik->get_effector_count();
	p_task->end_effectors.resize(effector_count);
	for (int32_t effector_i = 0; effector_i < effector_count; effector_i++) {
		p_task->end_effectors.write[effector_i].instance();
	}
	for (int32_t name_i = 0; name_i < p_task->end_effectors.size(); name_i++) {
		Ref<EWBIKBoneEffector> effector = p_task->ewbik->get_effector(name_i);
		if (effector.is_null()) {
			continue;
		}
		Ref<EWBIKBoneEffectorTransform> ee;
		ee.instance();
		// TODO Cache as object id
		Node *target_node = p_task->skeleton->get_node_or_null(effector->get_target_node());
		Transform node_xform;
		int32_t bone = p_task->skeleton->find_bone(effector->get_name());
		if (target_node) {
			Node3D *current_node = Object::cast_to<Node3D>(target_node);
			node_xform = p_task->skeleton->world_transform_to_global_pose(current_node->get_global_transform());
			node_xform = p_task->skeleton->global_pose_to_local_pose(bone, node_xform);
		}
		if (bone == -1) {
			continue;
		}
		node_xform = node_xform * effector->get_target_transform();
		ee->goal_transform = node_xform;
		int32_t constraint_i = p_task->ewbik->get_modification_stack()->get_skeleton()->find_bone(effector->get_name());
		Ref<KusudamaConstraint> constraint = p_task->ewbik->skeleton_ik_state->get_constraint(constraint_i);
		if (constraint.is_null()) {
			continue;
		}
		constraint->set_constraint_axes(ee->goal_transform);
		ee->effector_bone = bone;
		p_task->end_effectors.write[name_i] = ee;
	}

	Vector<Ref<EWBIKBoneChainTarget>> targets;
	targets.resize(p_task->end_effectors.size());
	for (int32_t effector_i = 0; effector_i < p_task->end_effectors.size(); effector_i++) {
		Ref<EWBIKBoneEffectorTransform> ee = p_task->end_effectors[effector_i];
		if (ee.is_null()) {
			ee.instance();
			p_task->end_effectors.write[effector_i] = ee;
		}
		Ref<EWBIKBoneChainTarget> target;
		target.instance();
		target->end_effector = ee;
		Ref<EWBIKSegmentedSkeleton3D> bone_chain_item = p_task->chain->chain_root->find_child(ee->effector_bone);
		if (bone_chain_item.is_null()) {
			continue;
		}
		target->chain_item = bone_chain_item;
		targets.write[effector_i] = target;
	}
	p_task->chain->targets = targets;
	p_task->chain->create_headings_arrays();
	if (!p_task->chain->targets.size()) {
		return;
	}
	solve_simple(p_task);
	// Strength is always full strength
	apply_bone_chains(1.0f, p_task->skeleton, p_task->chain->chain_root);
}

void SkeletonModification3DEWBIK::set_default_dampening(Ref<EWBIKSegmentedSkeleton3D> r_chain, float p_damp) {
	r_chain->dampening =
			MIN(Math_PI * 3.0f, MAX(Math::absf(std::numeric_limits<real_t>::epsilon()), Math::absf(p_damp)));
	update_armature_segments(r_chain);
}

void SkeletonModification3DEWBIK::update_armature_segments(Ref<EWBIKSegmentedSkeleton3D> r_chain) {
	r_chain->bone_segment_map.clear();
	recursively_update_bone_segment_map_from(r_chain, r_chain->chain_root);
}

void SkeletonModification3DEWBIK::update_optimal_rotation_to_target_descendants(Skeleton3D *p_skeleton, Ref<EWBIKSegmentedSkeleton3D> p_chain_item,
		float p_dampening, bool p_is_translate,
		Vector<Vector3> p_localized_effector_headings,
		Vector<Vector3> p_localized_target_headings,
		Vector<real_t> p_weights,
		Ref<QCP> p_qcp_orientation_aligner, int p_iteration,
		float p_total_iterations) {
	p_qcp_orientation_aligner->set_max_iterations(10);
	QuatIK qcp_rot = p_qcp_orientation_aligner->weighted_superpose(p_localized_effector_headings, p_localized_target_headings,
			p_weights, p_is_translate);
	Vector3 translate_by = p_qcp_orientation_aligner->get_translation();
	float bone_damp = p_chain_item->mod->get_state()->get_cos_half_dampen(p_chain_item->bone);
	if (p_dampening != -1) {
		bone_damp = p_dampening;
		qcp_rot.clamp_to_angle(bone_damp);
	} else {
		qcp_rot.clamp_to_quadrance_angle(bone_damp);
	}
	Transform xform;
	xform.basis.set_quat_scale(qcp_rot, Vector3(1.0, 1.0f, 1.0));
	xform.origin = translate_by;
	Ref<EWBIKState> state = p_chain_item->mod->get_state();
	Transform shadow_pose_local = state->get_shadow_pose_local(p_chain_item->bone);
	shadow_pose_local = shadow_pose_local * xform;
	state->set_shadow_bone_pose_local(p_chain_item->bone, shadow_pose_local);
	Ref<KusudamaConstraint> constraint = state->get_constraint(p_chain_item->bone);
	if (constraint.is_null()) {
		return;
	}
	xform.basis = shadow_pose_local.get_basis();
	xform.origin = shadow_pose_local.origin;
	p_chain_item->set_axes_to_be_snapped(xform, constraint->get_constraint_axes(), bone_damp);
	constraint->set_constraint_axes(constraint->get_constraint_axes().translated(translate_by));
}

void SkeletonModification3DEWBIK::recursively_update_bone_segment_map_from(Ref<EWBIKSegmentedSkeleton3D> r_chain,
		Ref<EWBIKSegmentedSkeleton3D> p_start_from) {
	for (int32_t child_i = 0; child_i < p_start_from->children.size(); child_i++) {
		r_chain->bone_segment_map.insert(p_start_from->children[child_i]->bone, p_start_from);
	}
}

void SkeletonModification3DEWBIK::update_optimal_rotation_to_target_descendants(Skeleton3D *p_skeleton,
		Ref<EWBIKSegmentedSkeleton3D> r_chain,
		Ref<EWBIKSegmentedSkeleton3D> p_for_bone,
		float p_dampening, bool p_translate,
		int p_stabilization_passes, int p_iteration,
		int p_total_iterations) {
	if (p_for_bone.is_null()) {
		return;
	}
	// print_line("Affected bone " + r_chain->skeleton->get_bone_name(p_for_bone->bone));
	p_for_bone->force_update_bone_children_transforms(r_chain);
	Ref<EWBIKState> state = r_chain->mod->get_state();
	BoneId bone = p_for_bone->bone;
	Transform bone_xform = state->get_shadow_pose_local(bone);
	Quat best_orientation = bone_xform.get_basis().get_rotation_quat();
	float new_dampening = -1;
	if (p_for_bone->parent_item == NULL || !r_chain->localized_target_headings.size()) {
		p_stabilization_passes = 0;
	}
	if (p_translate == true) {
		new_dampening = Math_PI;
	}

	update_target_headings(r_chain, r_chain->localized_target_headings, r_chain->weights, bone_xform);
	update_effector_headings(r_chain, r_chain->localized_effector_headings, bone_xform);

	float best_rmsd = 0.0f;
	Ref<QCP> qcp_convergence_check;
	qcp_convergence_check.instance();
	qcp_convergence_check->set_precision(FLT_EPSILON, FLT_EPSILON);
	float new_rmsd = 999999.0f;

	if (p_stabilization_passes > 0) {
		best_rmsd = get_manual_msd(r_chain->localized_effector_headings, r_chain->localized_target_headings,
				r_chain->weights);
	}

	for (int stabilization_i = 0; stabilization_i < p_stabilization_passes + 1; stabilization_i++) {
		update_optimal_rotation_to_target_descendants(
				p_skeleton,
				p_for_bone, new_dampening,
				p_translate,
				r_chain->localized_effector_headings,
				r_chain->localized_target_headings,
				r_chain->weights,
				qcp_convergence_check,
				p_iteration,
				p_total_iterations);

		if (p_stabilization_passes > 0) {
			update_effector_headings(r_chain, r_chain->localized_effector_headings, bone_xform);
			new_rmsd = get_manual_msd(r_chain->localized_effector_headings, r_chain->localized_target_headings,
					r_chain->weights);

			if (best_rmsd >= new_rmsd) {
				//if (p_for_bone->springy) {
				//	if (p_dampening != -1 || p_total_iterations != r_chain->get_default_iterations()) {
				//		float returnfullness = p_for_bone->constraint->get_pain();
				//		float dampened_angle = p_for_bone->get_stiffness() * p_dampening * returnfullness;
				//		float total_iterations_sq = p_total_iterations * p_total_iterations;
				//		float scaled_dampened_angle = dampened_angle *
				//									  ((total_iterations_sq - (p_iteration * p_iteration)) /
				//											  total_iterations_sq);
				//		float cos_half_angle = Math::cos(0.5f * scaled_dampened_angle);
				//		p_for_bone->set_axes_to_returned(p_skeleton->get_bone_global_pose(p_for_bone->bone), bone_xform,
				//				p_for_bone->constraint->get_limiting_axes(), cos_half_angle,
				//				scaled_dampened_angle);
				//	} else {
				//		p_for_bone->set_axes_to_returned(p_skeleton->get_bone_global_pose(p_for_bone->bone), bone_xform,
				//				p_for_bone->constraint->get_limiting_axes(),
				//				p_for_bone->cos_half_returnful_dampened[p_iteration],
				//				p_for_bone->half_returnful_dampened[p_iteration]);
				//	}
				//	update_effector_headings(r_chain, r_chain->localized_effector_headings, bone_xform);
				//	new_rmsd = get_manual_msd(r_chain->localized_effector_headings, r_chain->localized_target_headings,
				//			r_chain->weights);
				//}
				best_orientation = bone_xform.basis.get_rotation_quat();
				best_rmsd = new_rmsd;
				break;
			}
		} else {
			break;
		}
	}
	if (p_stabilization_passes > 0) {
		Transform pose;
		pose.basis.set_quat_scale(bone_xform.basis.get_rotation_quat(), pose.basis.get_scale());
		pose.origin = bone_xform.origin;
		state->set_shadow_bone_pose_local(p_for_bone->bone, pose);
	}
}

float SkeletonModification3DEWBIK::get_manual_msd(Vector<Vector3> &r_localized_effector_headings,
		Vector<Vector3> &r_localized_target_headings,
		const Vector<real_t> &p_weights) {
	float manual_rmsd = 0.0f;
	float wsum = 0.0f;
	for (int i = 0; i < r_localized_target_headings.size(); i++) {
		float xd = r_localized_target_headings[i].x - r_localized_effector_headings[i].x;
		float yd = r_localized_target_headings[i].y - r_localized_effector_headings[i].y;
		float zd = r_localized_target_headings[i].z - r_localized_effector_headings[i].z;
		float mag_sq = p_weights[i] * (xd * xd + yd * yd + zd * zd);
		manual_rmsd += mag_sq;
		wsum += p_weights[i];
	}
	manual_rmsd /= wsum;
	return manual_rmsd;
}

void SkeletonModification3DEWBIK::update_target_headings(Ref<EWBIKSegmentedSkeleton3D> r_chain, Vector<Vector3> &r_localized_target_headings,
		Vector<real_t> &p_weights, Transform p_bone_xform) {
	int hdx = 0;
	Ref<EWBIKState> state = r_chain->mod->get_state();
	for (int target_i = 0; target_i < r_chain->targets.size(); target_i++) {
		Ref<EWBIKBoneChainTarget> bct = r_chain->targets[target_i];
		if (bct.is_null()) {
			continue;
		}
		Ref<EWBIKBoneEffectorTransform> ee = bct->end_effector;
		if (ee.is_null()) {
			continue;
		}
		Ref<EWBIKSegmentedSkeleton3D> sb = r_chain->find_child(ee->effector_bone);
		if (sb.is_null()) {
			continue;
		}
		Ref<KusudamaConstraint> constraint = state->get_constraint(sb->bone);
		if (constraint.is_null()) {
			continue;
		}
		Transform target_axes = constraint->get_constraint_axes();
		r_localized_target_headings.write[hdx] = target_axes.origin;
		uint8_t modeCode = r_chain->targets[target_i]->get_mode_code();
		Transform pose = state->get_shadow_pose_local(sb->bone);
		Vector3 origin = pose.origin;
		// Remove hacks 2020-01-01
		// Vector3 godot_to_libgdx = Vector3(-1.0f, 1.0f, -1.0f);
		// origin += godot_to_libgdx;
		hdx++;
		if ((modeCode & EWBIKBoneChainTarget::XDir) != 0) {
			r_localized_target_headings.write[hdx] += origin * x_orientation + target_axes.origin;
			r_localized_target_headings.write[hdx + 1] += -r_localized_target_headings.write[hdx];
			hdx += 2;
		}
		if ((modeCode & EWBIKBoneChainTarget::YDir) != 0) {
			r_localized_target_headings.write[hdx] += origin * y_orientation + target_axes.origin;
			r_localized_target_headings.write[hdx + 1] += -r_localized_target_headings.write[hdx];
			hdx += 2;
		}
		if ((modeCode & EWBIKBoneChainTarget::ZDir) != 0) {
			r_localized_target_headings.write[hdx] += origin * z_orientation + target_axes.origin;
			r_localized_target_headings.write[hdx + 1] += -r_localized_target_headings.write[hdx];
			hdx += 2;
		}
	}
}

void SkeletonModification3DEWBIK::update_effector_headings(Ref<EWBIKSegmentedSkeleton3D> r_chain, Vector<Vector3> &r_localized_effector_headings,
		Transform p_bone_xform) {
	int hdx = 0;
	Ref<EWBIKState> state = r_chain->mod->get_state();
	for (int target_i = 0; target_i < r_chain->targets.size(); target_i++) {
		Ref<EWBIKBoneChainTarget> bct = r_chain->targets[target_i];
		if (bct.is_null()) {
			continue;
		}
		Ref<EWBIKBoneEffectorTransform> ee = bct->end_effector;
		if (ee.is_null()) {
			continue;
		}
		Ref<EWBIKSegmentedSkeleton3D> sb = r_chain->find_child(ee->effector_bone);
		if (sb.is_null()) {
			continue;
		}
		// int bone = r_chain->targets[target_i]->end_effector->effector_bone;
		Transform pose = state->get_shadow_pose_local(sb->bone);
		Vector3 origin = pose.origin;
		Vector3 godot_to_libgdx = Vector3(-1.0f, 1.0f, -1.0f);
		r_localized_effector_headings.write[hdx] = origin;
		origin += godot_to_libgdx;
		uint8_t modeCode = r_chain->targets[target_i]->get_mode_code();
		hdx++;
		// Transform target_axes = sb->constraint->get_constraint_axes();
		if ((modeCode & EWBIKBoneChainTarget::XDir) != 0) {
			r_localized_effector_headings.write[hdx] += (origin + p_bone_xform.origin) * x_orientation;
			r_localized_effector_headings.write[hdx + 1] += -r_localized_effector_headings.write[hdx];
			hdx += 2;
		}
		if ((modeCode & EWBIKBoneChainTarget::YDir) != 0) {
			r_localized_effector_headings.write[hdx] += (origin + p_bone_xform.origin) * y_orientation;
			r_localized_effector_headings.write[hdx + 1] += -r_localized_effector_headings.write[hdx];
			hdx += 2;
		}
		if ((modeCode & EWBIKBoneChainTarget::ZDir) != 0) {
			r_localized_effector_headings.write[hdx] += (origin + p_bone_xform.origin) * z_orientation;
			r_localized_effector_headings.write[hdx + 1] += -r_localized_effector_headings.write[hdx];
			hdx += 2;
		}
	}
}

int EWBIKSegmentedSkeleton3D::get_default_iterations() const {
	return ik_iterations;
}

void EWBIKSegmentedSkeleton3D::rootwardly_update_falloff_cache_from(Ref<EWBIKSegmentedSkeleton3D> p_current) {
	Ref<EWBIKSegmentedSkeleton3D> current = p_current;
	while (current.is_valid()) {
		current->chain_root->create_headings_arrays();
		current = current->parent_item;
	}
}

void EWBIKSegmentedSkeleton3D::recursively_create_penalty_array(Ref<EWBIKSegmentedSkeleton3D> from,
		Vector<Vector<real_t>> &r_weight_array,
		Vector<Ref<EWBIKSegmentedSkeleton3D>>
				pin_sequence,
		float current_falloff) {
	if (current_falloff == 0) {
		return;
	} else {
		for (int32_t target_i = 0; target_i < from->targets.size(); target_i++) {
			Ref<EWBIKBoneChainTarget> target = from->targets[target_i];
			if (target.is_null()) {
				continue;
			}
			Vector<real_t> inner_weight_array;
			uint8_t mode_code = target->get_mode_code();
			inner_weight_array.push_back(target->get_target_weight() * current_falloff);
			float max_target_weight = 0.0f;
			if ((mode_code & EWBIKBoneChainTarget::XDir) != 0)
				max_target_weight = MAX(max_target_weight, target->get_x_priority());
			if ((mode_code & EWBIKBoneChainTarget::YDir) != 0)
				max_target_weight = MAX(max_target_weight, target->get_y_priority());
			if ((mode_code & EWBIKBoneChainTarget::ZDir) != 0)
				max_target_weight = MAX(max_target_weight, target->get_z_priority());

			if (max_target_weight == 0.0f)
				max_target_weight = 1.0f;

			max_target_weight = 1.0f;

			if ((mode_code & EWBIKBoneChainTarget::XDir) != 0) {
				float sub_target_weight = target->get_target_weight() * (target->get_x_priority() / max_target_weight) * current_falloff;
				inner_weight_array.push_back(sub_target_weight);
				inner_weight_array.push_back(sub_target_weight);
			}
			if ((mode_code & EWBIKBoneChainTarget::YDir) != 0) {
				float sub_target_weight = target->get_target_weight() * (target->get_y_priority() / max_target_weight) * current_falloff;
				inner_weight_array.push_back(sub_target_weight);
				inner_weight_array.push_back(sub_target_weight);
			}
			if ((mode_code & EWBIKBoneChainTarget::ZDir) != 0) {
				float sub_target_weight = target->get_target_weight() * (target->get_z_priority() / max_target_weight) * current_falloff;
				inner_weight_array.push_back(sub_target_weight);
				inner_weight_array.push_back(sub_target_weight);
			}
			pin_sequence.push_back(target->for_bone());
			r_weight_array.push_back(inner_weight_array);
			float this_falloff = target.is_null() ? 1.0f : target->get_depth_falloff();
			for (int32_t child_i = 0; child_i < children.size(); child_i++) {
				children.write[child_i]->recursively_create_penalty_array(from, r_weight_array, pin_sequence, current_falloff * this_falloff);
			}
		}
	}
}

void EWBIKSegmentedSkeleton3D::create_headings_arrays() {
	Vector<Vector<real_t>> penalty_array;
	Vector<Ref<EWBIKSegmentedSkeleton3D>> target_sequence;
	chain_root->recursively_create_penalty_array(this, penalty_array, target_sequence, 1.0f);
	Vector<Ref<EWBIKSegmentedSkeleton3D>> target_bones;
	target_bones.resize(target_sequence.size());
	int total_headings = 0;
	for (int32_t penalty_i = 0; penalty_i < penalty_array.size(); penalty_i++) {
		total_headings += penalty_array[penalty_i].size();
	}
	for (int pin_i = 0; pin_i < target_sequence.size(); pin_i++) {
		target_bones.write[pin_i] = target_sequence[pin_i];
	}
	localized_effector_headings.resize(total_headings);
	localized_target_headings.resize(total_headings);
	weights.resize(total_headings);
	int current_heading = 0;
	for (int32_t array_i = 0; array_i < penalty_array.size(); array_i++) {
		for (int32_t penalty_i = 0; penalty_i < penalty_array[array_i].size(); penalty_i++) {
			weights.write[current_heading] = penalty_array[array_i][penalty_i];
			localized_effector_headings.write[current_heading] = Vector3();
			localized_target_headings.write[current_heading] = Vector3();
			current_heading++;
		}
	}
}

void EWBIKSegmentedSkeleton3D::force_update_bone_children_transforms(Ref<EWBIKSegmentedSkeleton3D> p_current_chain) {
	Ref<EWBIKState> state = p_current_chain->mod->get_state();
	BoneId bone = p_current_chain->bone;
	state->set_bone_dirty(bone, true);
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bones = p_current_chain->get_bones();
	ERR_FAIL_COND(!p_current_chain->is_chain_active());
	for (int32_t bone_i = 0; bone_i < bones.size(); bone_i++) {
		state->set_bone_dirty(bone, false);
		if (bones[bone_i]->parent_item.is_valid()) {
			state->set_bone_dirty(bone_i, true);
		}
	}
	Vector<Ref<EWBIKSegmentedSkeleton3D>> bone_chains = p_current_chain->get_child_chains();
	for (int32_t i = 0; i < bone_chains.size(); i++) {
		force_update_bone_children_transforms(bone_chains[i]);
	}
}

void EWBIKBoneChainTarget::set_parent_target(EWBIKBoneChainTarget *parent) {
	if (parent_target != NULL) {
		parent_target->remove_child_target(this);
	}
	//set the parent to the global axes if the user
	//tries to set the pin to be its own parent

	if (parent != NULL) {
		parent->add_child_target(this);
		parent_target = parent;
	}
}

void EWBIKBoneChainTarget::remove_child_target(EWBIKBoneChainTarget *child) {
	int32_t target_i = child_targets.find(child);
	if (target_i != -1) {
		child_targets.remove(target_i);
	}
}

void EWBIKBoneChainTarget::add_child_target(EWBIKBoneChainTarget *newChild) {
	if (newChild->is_ancestor_of(this)) {
		set_parent_target(newChild->get_parent_target());
	}
	if (child_targets.find(newChild) != -1) {
		child_targets.push_back(newChild);
	}
}

EWBIKBoneChainTarget *EWBIKBoneChainTarget::get_parent_target() {
	return parent_target;
}

bool EWBIKBoneChainTarget::is_ancestor_of(EWBIKBoneChainTarget *potentialDescendent) {
	bool result = false;
	EWBIKBoneChainTarget *cursor = potentialDescendent->get_parent_target();
	while (cursor) {
		if (cursor == this) {
			result = true;
			break;
		} else {
			cursor = cursor->parent_target;
		}
	}
	return result;
}

float EWBIKBoneChainTarget::get_target_weight() {
	return target_weight;
}

float EWBIKState::get_stiffness(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);
	return bones[p_bone].sim_local_ik_node.get_stiffness();
}

void EWBIKState::set_stiffness(int32_t p_bone, float p_stiffness_scalar) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.set_stiffness(p_stiffness_scalar);
}

float EWBIKState::get_height(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);
	return bones[p_bone].sim_local_ik_node.get_height();
}

void EWBIKState::set_height(int32_t p_bone, float p_height) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	return bones.write[p_bone].sim_local_ik_node.set_height(p_height);
}

Ref<KusudamaConstraint> EWBIKState::get_constraint(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), nullptr);
	return bones[p_bone].get_constraint();
}

void EWBIKState::set_constraint(int32_t p_bone, Ref<KusudamaConstraint> p_constraint) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].set_constraint(p_constraint);
}

void EWBIKState::init(Ref<SkeletonModification3DEWBIK> p_mod) {
	ERR_FAIL_COND(p_mod.is_null());
	mod = p_mod;
	Ref<SkeletonModificationStack3D> stack = p_mod->get_modification_stack();
	ERR_FAIL_COND(stack.is_null());
	Skeleton3D *skeleton = stack->get_skeleton();
	if (!skeleton) {
		return;
	}
	set_bone_count(skeleton->get_bone_count());
	for (int32_t bone_i = 0; bone_i < skeleton->get_bone_count(); bone_i++) {
		set_parent(bone_i, skeleton->get_bone_parent(bone_i));
		set_stiffness(bone_i, -1);
		set_height(bone_i, -1);
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		constraint->set_name(skeleton->get_bone_name(bone_i));
		set_constraint(bone_i, constraint);
	}
	_update_process_order();
}

void EWBIKState::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "bone_count"));
	for (int bone_i = 0; bone_i < get_bone_count(); bone_i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, "bone/" + itos(bone_i) + "/name"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/stiffness"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/height"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/twist_min_angle"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/twist_range"));
		p_list->push_back(PropertyInfo(Variant::INT, "bone/" + itos(bone_i) + "/direction_count", PROPERTY_HINT_RANGE, "0,65535,1"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, "bone/" + itos(bone_i) + "/constraint_axes"));
		Ref<KusudamaConstraint> kusudama = get_constraint(bone_i);
		if (kusudama.is_null()) {
			continue;
		}
		for (int j = 0; j < kusudama->get_direction_count(); j++) {
			p_list->push_back(PropertyInfo(Variant::FLOAT, "bone/" + itos(bone_i) + "/direction" + "/" + itos(j) + "/radius", PROPERTY_HINT_RANGE, "0,65535,or_greater"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "bone/" + itos(bone_i) + "/direction" + "/" + itos(j) + "/control_point"));
		}
	}
}

bool EWBIKState::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "bone_count") {
		r_ret = get_bone_count();
		return true;
	} else if (name.begins_with("bone/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		if (what == "stiffness") {
			r_ret = get_stiffness(index);
			return true;
		} else if (what == "height") {
			r_ret = get_height(index);
			return true;
		}
		Ref<KusudamaConstraint> kusudama = get_constraint(index);
		if (kusudama.is_null()) {
			return false;
		}
		if (what == "name") {
			r_ret = kusudama->get_name();
			return true;
		} else if (what == "twist_min_angle") {
			r_ret = kusudama->get_twist_constraint()->get_min_twist_angle();
			return true;
		} else if (what == "twist_range") {
			r_ret = kusudama->get_twist_constraint()->get_range();
			return true;
		} else if (what == "limiting") {
			r_ret = kusudama->get_twist_constraint()->get_range();
			return true;
		} else if (what == "direction_count") {
			r_ret = kusudama->get_direction_count();
			return true;
		} else if (what == "constraint_axes") {
			r_ret = kusudama->get_constraint_axes();
			return true;
		} else if (what == "direction") {
			int direction_index = name.get_slicec('/', 3).to_int();
			ERR_FAIL_INDEX_V(direction_index, kusudama->get_direction_count(), false);
			Ref<DirectionConstraint> direction = kusudama->get_direction(direction_index);
			if (direction.is_null()) {
				return false;
			}
			String direction_what = name.get_slicec('/', 4);
			if (direction_what == "radius") {
				r_ret = direction->get_radius();
				return true;
			} else if (direction_what == "control_point") {
				r_ret = direction->get_control_point();
				return true;
			}
		}
	}
	return false;
}

bool EWBIKState::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name == "bone_count") {
		set_bone_count(p_value);
		return true;
	} else if (name.begins_with("bone/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, bone_count, false);
		if (what == "stiffness") {
			set_stiffness(index, p_value);
			_change_notify();
		} else if (what == "height") {
			set_height(index, p_value);
			_change_notify();
		}
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		set_constraint(index, constraint);
		if (what == "name") {
			constraint->set_name(p_value);
			_change_notify();
			return true;
		} else if (what == "twist_min_angle") {
			Ref<TwistConstraint> twist = constraint->get_twist_constraint();
			twist->set_min_twist_angle(p_value);
			constraint->set_twist_constraint(twist);
			_change_notify();
			return true;
		} else if (what == "twist_range") {
			Ref<TwistConstraint> twist = constraint->get_twist_constraint();
			twist->set_range(p_value);
			constraint->set_twist_constraint(twist);
			_change_notify();
			return true;
		} else if (what == "constraint_axes") {
			constraint->set_constraint_axes(p_value);
			_change_notify();
			return true;
		} else if (what == "direction_count") {
			constraint->set_direction_count(p_value);
			_change_notify();
			return true;
		} else if (what == "direction") {
			int direction_index = name.get_slicec('/', 3).to_int();
			ERR_FAIL_INDEX_V(direction_index, constraint->get_direction_count(), false);
			Ref<DirectionConstraint> direction = constraint->get_direction(direction_index);
			if (direction.is_null()) {
				direction.instance();
				constraint->set_direction(direction_index, direction);
			}
			String direction_what = name.get_slicec('/', 4);
			if (direction_what == "radius") {
				direction->set_radius(p_value);
				_change_notify();
			} else if (direction_what == "control_point") {
				direction->set_control_point(p_value);
				// TODO
				// constraint->optimize_limiting_axes();
				_change_notify();
			} else {
				return false;
			}
			return true;
		}
		return true;
	}
	return false;
}
void EWBIKState::set_bone_count(int32_t p_bone_count) {
	bone_count = p_bone_count;
	bones.resize(p_bone_count);
	for (int32_t bone_i = 0; bone_i < p_bone_count; bone_i++) {
		bones.write[bone_i] = ShadowBone3D();
	}
}

Transform EWBIKState::get_shadow_pose_global(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].sim_local_ik_node.get_global().get_transform();
}
Transform EWBIKState::get_shadow_pose_local(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].sim_local_ik_node.get_local().get_transform();
}
void EWBIKState::set_shadow_bone_pose_local(int p_bone, const Transform &value) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.set_local(value);
	mark_dirty(p_bone);
}
Transform EWBIKState::get_shadow_constraint_pose_global(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].sim_constraint_ik_node.get_global().get_transform();
}
void EWBIKState::mark_dirty(int32_t p_bone) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.dirty = true;
	bones.write[p_bone].sim_constraint_ik_node.dirty = true;
	update_skeleton();
}
bool EWBIKState::is_dirty(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), true);
	return bones[p_bone].sim_local_ik_node.dirty || bones[p_bone].sim_constraint_ik_node.dirty;
}
void EWBIKState::_update_process_order() {
	ShadowBone3D *bonesptr = bones.ptrw();
	int len = bones.size();

	parentless_bones.clear();

	for (int i = 0; i < len; i++) {
		if (bonesptr[i].sim_local_ik_node.parent >= len) {
			//validate this just in case
			ERR_PRINT("Bone " + itos(i) + " has invalid parent: " + itos(bonesptr[i].sim_local_ik_node.parent));
			bonesptr[i].sim_local_ik_node.parent = -1;
		}
		bonesptr[i].sim_local_ik_node.child_bones.clear();

		if (bonesptr[i].sim_local_ik_node.parent != -1) {
			int parent_bone_idx = bonesptr[i].sim_local_ik_node.parent;

			// Check to see if this node is already added to the parent:
			if (bonesptr[parent_bone_idx].sim_local_ik_node.child_bones.find(i) < 0) {
				// Add the child node
				bonesptr[parent_bone_idx].sim_local_ik_node.child_bones.push_back(i);
			} else {
				ERR_PRINT("IkNode3D parenthood graph is cyclic");
			}
		} else {
			parentless_bones.push_back(i);
		}
	}
}
void EWBIKState::rotate_by(int32_t p_bone, Quat addRotation) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	if (bonesptr[p_bone].sim_local_ik_node.parent != -1) {
		Quat newRot = bonesptr[bonesptr[p_bone].sim_local_ik_node.parent].sim_local_ik_node.get_global_ik_basis().get_local_of_rotation(addRotation);
		bonesptr[p_bone].sim_local_ik_node.get_local_ik_basis().rotate_by(newRot);
	} else {
		bonesptr[p_bone].sim_local_ik_node.get_local_ik_basis().rotate_by(addRotation);
	}
	mark_dirty(p_bone);
}
void EWBIKState::translate_to(int32_t p_bone, Vector3 p_target) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	if (bonesptr[p_bone].sim_local_ik_node.parent != -1) {
		bonesptr[p_bone].sim_local_ik_node.pose_local.translate_to(bonesptr[bonesptr[p_bone].sim_local_ik_node.parent].sim_local_ik_node.get_global().get_local_of(p_target));
		mark_dirty(p_bone);
	} else {
		bonesptr[p_bone].sim_local_ik_node.pose_local.translate_to(p_target);
		mark_dirty(p_bone);
	}
}
Ray EWBIKState::get_ray_x(int32_t p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Ray());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	return bonesptr[p_bone].sim_local_ik_node.get_global_ik_basis().get_x_ray();
}
Ray EWBIKState::get_ray_y(int32_t p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Ray());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	return bonesptr[p_bone].sim_local_ik_node.get_global_ik_basis().get_y_ray();
}
Ray EWBIKState::get_ray_z(int32_t p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Ray());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	return bonesptr[p_bone].sim_local_ik_node.get_global_ik_basis().get_z_ray();
}
void EWBIKState::rotate_about_x(int32_t p_bone, float angle) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	Quat xRot = Quat(bonesptr[p_bone].sim_local_ik_node.get_global_ik_basis().get_x_heading(), angle);
	rotate_by(p_bone, xRot);
	mark_dirty(p_bone);
}
void EWBIKState::rotate_about_y(int32_t p_bone, float angle) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	Quat yRot = Quat(bonesptr[p_bone].sim_local_ik_node.get_global_ik_basis().get_y_heading(), angle);
	rotate_by(p_bone, yRot);
	mark_dirty(p_bone);
}
void EWBIKState::rotate_about_z(int32_t p_bone, float angle) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	bonesptr[p_bone].sim_local_ik_node.update_global();
	Quat zRot = Quat(bonesptr[p_bone].sim_local_ik_node.get_global_ik_basis().get_z_heading(), angle);
	rotate_by(p_bone, zRot);
	mark_dirty(p_bone);
}
void EWBIKState::force_update_bone_children_transforms(int p_bone_idx) {
	ERR_FAIL_INDEX(p_bone_idx, bones.size());
	ShadowBone3D *bonesptr = bones.ptrw();
	List<int> bones_to_process = List<int>();
	bones_to_process.push_back(p_bone_idx);

	while (bones_to_process.size() > 0) {
		int current_bone_idx = bones_to_process[0];
		bones_to_process.erase(current_bone_idx);
		IKNode3D &b = bonesptr[current_bone_idx].sim_local_ik_node;
		IKBasis pose = bones[p_bone_idx].sim_local_ik_node.get_local();
		if (b.parent >= 0) {
			b.pose_global = bonesptr[b.parent].sim_local_ik_node.pose_global * pose;
		} else {
			b.pose_global = pose;
		}

		// Add the bone's children to the list of bones to be processed
		int child_bone_size = b.child_bones.size();
		for (int i = 0; i < child_bone_size; i++) {
			bones_to_process.push_back(b.child_bones[i]);
		}
	}
}
void EWBIKState::update_skeleton() {
	// Update bone transforms
	int parentless_bones_size = parentless_bones.size();
	for (int i = 0; i < parentless_bones_size; i++) {
		bones.write[parentless_bones[i]].sim_local_ik_node.dirty = false;
		bones.write[parentless_bones[i]].sim_constraint_ik_node.dirty = false;
		force_update_bone_children_transforms(parentless_bones[i]);
	}
}
void EWBIKState::set_parent(int32_t p_bone, int32_t p_parent) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ERR_FAIL_COND(p_parent != -1 && (p_parent < 0));

	bones.write[p_bone].sim_local_ik_node.parent = p_parent;
	bones.write[p_bone].sim_constraint_ik_node.parent = p_parent;
	mark_dirty(p_bone);
}
int32_t EWBIKState::get_parent(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);
	return bones[p_bone].sim_local_ik_node.parent;
}
void EWBIKState::align_shadow_bone_globals_to(int p_bone, Transform p_target) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	force_update_bone_children_transforms(p_bone);
	if (get_parent(p_bone) != -1) {
		// TODO Set local of?
		Transform shadow_pose_global = get_shadow_pose_global(get_parent(p_bone));
		shadow_pose_global = global_shadow_pose_to_local_pose(p_bone, shadow_pose_global);
		set_shadow_bone_pose_local(p_bone, p_target);
	} else {
		Transform shadow_pose_global = get_shadow_pose_global(get_parent(p_bone));
		set_shadow_bone_pose_local(p_bone, shadow_pose_global);
	}
	mark_dirty(p_bone);
	force_update_bone_children_transforms(p_bone);
}
void EWBIKState::align_shadow_constraint_globals_to(int p_bone, Transform p_target) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	force_update_bone_children_transforms(p_bone);
	if (get_parent(p_bone) != -1) {
		Transform shadow_constraint_pose_global = get_shadow_pose_global(get_parent(p_bone));
		shadow_constraint_pose_global = global_constraint_pose_to_local_pose(p_bone, shadow_constraint_pose_global);
		set_shadow_constraint_pose_local(p_bone, p_target);
	} else {
		Transform shadow_pose_global = get_shadow_pose_global(get_parent(p_bone));
		set_shadow_constraint_pose_local(p_bone, shadow_pose_global);
	}
	mark_dirty(p_bone);
	force_update_bone_children_transforms(p_bone);
}
void EWBIKState::set_shadow_constraint_pose_local(int p_bone, const Transform &value) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_constraint_ik_node.set_local(value);
	mark_dirty(p_bone);
}
Transform EWBIKState::get_shadow_constraint_pose_local(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].sim_constraint_ik_node.get_local().get_transform();
}
Transform EWBIKState::global_constraint_pose_to_local_pose(int p_bone_idx, Transform p_global_pose) {
	if (bones[p_bone_idx].sim_constraint_ik_node.parent >= 0) {
		int parent_bone_idx = bones[p_bone_idx].sim_constraint_ik_node.parent;
		Transform conversion_transform = bones[parent_bone_idx].sim_constraint_ik_node.get_global().get_transform();
		return conversion_transform.affine_inverse() * p_global_pose;
	} else {
		return p_global_pose;
	}
}
Transform EWBIKState::global_shadow_pose_to_local_pose(int p_bone_idx, Transform p_global_pose) {
	if (bones[p_bone_idx].sim_local_ik_node.parent >= 0) {
		int parent_bone_idx = bones[p_bone_idx].sim_local_ik_node.parent;
		Transform conversion_transform = bones[parent_bone_idx].sim_local_ik_node.get_global().get_transform();
		return conversion_transform.affine_inverse() * p_global_pose;
	} else {
		return p_global_pose;
	}
}
float EWBIKState::get_cos_half_dampen(int32_t p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), 0.0f);
	return bones[p_bone].cos_half_dampen;
}
void EWBIKState::set_cos_half_dampen(int32_t p_bone, float p_cos_half_dampen) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].cos_half_dampen = p_cos_half_dampen;
}
// Vector<float> EWBIKState::get_half_returnful_dampened(int32_t p_bone) const {
// 	ERR_FAIL_INDEX_V(p_bone, bones.size(), Vector<float>());
// 	return bones[p_bone].half_returnfullness_dampened;
// }
// Vector<float> EWBIKState::get_cos_half_returnful_dampened(int32_t p_bone) const {
// 	ERR_FAIL_INDEX_V(p_bone, bones.size(), Vector<float>());
// 	return bones[p_bone].cos_half_returnfullness_dampened;
// }
// void EWBIKState::set_cos_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened) {
// 	ERR_FAIL_INDEX(p_bone, bones.size());
// 	bones.write[p_bone].cos_half_returnfullness_dampened = p_dampened;
// }
// void EWBIKState::set_half_returnfullness_dampened(int32_t p_bone, Vector<float> p_dampened) {
// 	ERR_FAIL_INDEX(p_bone, bones.size());
// 	bones.write[p_bone].half_returnfullness_dampened = p_dampened;
// }void EWBIKState::ShadowBone3D::set_constraint(Ref<KusudamaConstraint> p_constraint) {
void EWBIKState::ShadowBone3D::set_constraint(Ref<KusudamaConstraint> p_constraint) {
	constraint = p_constraint;
}
Ref<KusudamaConstraint> EWBIKState::ShadowBone3D::get_constraint() const {
	return constraint;
}
void EWBIKState::ShadowBone3D::update_cos_dampening(int p_default_iterations, float p_default_dampening) {
	float predampening = 1.0f - sim_local_ik_node.stiffness;
	float dampening = sim_constraint_ik_node.parent == -1 ? Math_PI : predampening * p_default_dampening;
	cos_half_dampen = Math::cos(dampening / 2.0f);
	if (constraint.is_valid() && constraint->get_pain() != 0.0f) {
		springy = true;
		populate_return_dampening_iteration_array(p_default_iterations, p_default_dampening);
	} else {
		springy = false;
	}
}
void EWBIKState::ShadowBone3D::populate_return_dampening_iteration_array(int p_default_iterations, float p_default_dampening) {
	float predampen = 1.0f - sim_local_ik_node.stiffness;
	float dampening = sim_constraint_ik_node.parent == -1 ? Math_PI : predampen * p_default_dampening;
	float returnful = constraint->get_pain();
	float falloff = 0.2f;
	half_returnful_dampened.resize(p_default_iterations);
	cos_half_returnful_dampened.resize(p_default_iterations);
	float iterations_pow = Math::pow(p_default_iterations, falloff * p_default_iterations * returnful);
	for (int32_t iter_i = 0; iter_i < p_default_iterations; iter_i++) {
		float iteration_scalar =
				((iterations_pow)-Math::pow(iter_i, falloff * p_default_iterations * returnful)) / (iterations_pow);
		float iteration_return_clamp = iteration_scalar * returnful * dampening;
		float cos_iteration_return_clamp = Math::cos(iteration_return_clamp / 2.0f);
		half_returnful_dampened.write[iter_i] = iteration_return_clamp;
		cos_half_returnful_dampened.write[iter_i] = cos_iteration_return_clamp;
	}
}
void EWBIKState::set_bone_dirty(int p_bone, bool p_dirty) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].sim_local_ik_node.dirty = p_dirty;
	bones.write[p_bone].sim_constraint_ik_node.dirty = p_dirty;
}
bool EWBIKState::get_bone_dirty(int p_bone) const {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), true);
	return bones[p_bone].sim_local_ik_node.dirty || bones[p_bone].sim_constraint_ik_node.dirty;
}