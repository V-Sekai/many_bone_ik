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

#include "skeleton_modification_3d_dmik.h"
#include "direction_constraint.h"
#include "kusudama_constraint.h"
#include "scene/3d/skeleton_3d.h"

void SkeletonModification3DDMIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_constraint_count", "constraint_count"),
			&SkeletonModification3DDMIK::set_constraint_count);
	ClassDB::bind_method(D_METHOD("get_constraint_count"), &SkeletonModification3DDMIK::get_constraint_count);
	ClassDB::bind_method(D_METHOD("get_effector_count"), &SkeletonModification3DDMIK::get_effector_count);
	ClassDB::bind_method(D_METHOD("set_effector_count", "count"),
			&SkeletonModification3DDMIK::set_effector_count);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node", "target_transform", "budget"), &SkeletonModification3DDMIK::add_effector);
	ClassDB::bind_method(D_METHOD("get_effector", "index"), &SkeletonModification3DDMIK::get_effector);
	ClassDB::bind_method(D_METHOD("get_constraint", "index"), &SkeletonModification3DDMIK::get_constraint);
	ClassDB::bind_method(D_METHOD("set_effector", "index", "effector"), &SkeletonModification3DDMIK::set_effector);
	ClassDB::bind_method(D_METHOD("set_constraint", "index", "constraint"), &SkeletonModification3DDMIK::set_constraint);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "constraint_count", PROPERTY_HINT_RANGE, "0,65535,or_greater"), "set_constraint_count", "get_constraint_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "effector_count", PROPERTY_HINT_RANGE, "0,65535,or_greater"), "set_effector_count", "get_effector_count");
	ADD_SIGNAL(MethodInfo("ik_changed"));
}

void SkeletonModification3DDMIK::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < constraint_count; i++) {
		p_list->push_back(PropertyInfo(Variant::STRING, "kusudama_constraints/" + itos(i) + "/name"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "kusudama_constraints/" + itos(i) + "/twist_min_angle"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, "kusudama_constraints/" + itos(i) + "/twist_range"));
		p_list->push_back(PropertyInfo(Variant::INT, "kusudama_constraints/" + itos(i) + "/direction_count"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, "kusudama_constraints/" + itos(i) + "/constraint_axes"));
		ERR_CONTINUE(get_constraint(i).is_null());
		for (int j = 0; j < get_constraint(i)->get_direction_count(); j++) {
			p_list->push_back(PropertyInfo(Variant::FLOAT, "kusudama_constraints/" + itos(i) + "/direction" + "/" + itos(j) + "/radius", PROPERTY_HINT_RANGE, "0,65535,or_greater"));
			p_list->push_back(PropertyInfo(Variant::VECTOR3, "kusudama_constraints/" + itos(i) + "/direction" + "/" + itos(j) + "/control_point"));
		}
	}

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
}

bool SkeletonModification3DDMIK::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	if (name == "root_bone") {
		r_ret = get_root_bone();
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
	} else if (name.begins_with("kusudama_constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, get_constraint_count(), false);
		if (what == "name") {
			ERR_FAIL_COND_V(get_constraint(index).is_null(), false);
			r_ret = get_constraint(index)->get_name();
			return true;
		} else if (what == "twist_min_angle") {
			ERR_FAIL_COND_V(get_constraint(index).is_null(), false);
			r_ret = get_constraint(index)->get_twist_constraint()->get_min_twist_angle();
			return true;
		} else if (what == "twist_range") {
			ERR_FAIL_COND_V(get_constraint(index).is_null(), false);
			r_ret = get_constraint(index)->get_twist_constraint()->get_range();
			return true;
		} else if (what == "limiting") {
			ERR_FAIL_COND_V(get_constraint(index).is_null(), false);
			r_ret = get_constraint(index)->get_twist_constraint()->get_range();
			return true;
		} else if (what == "constraint_axes") {
			ERR_FAIL_COND_V(get_constraint(index).is_null(), false);
			r_ret = get_constraint(index)->get_constraint_axes();
			return true;
		} else if (what == "direction") {
			int direction_index = name.get_slicec('/', 3).to_int();
			ERR_FAIL_INDEX_V(direction_index, get_constraint(index)->get_direction_count(), false);
			if (get_constraint(index)->get_direction(direction_index).is_null()) {
				return false;
			}
			String direction_what = name.get_slicec('/', 4);
			if (direction_what == "radius") {
				r_ret = get_constraint(index)->get_direction(direction_index)->get_radius();
				return true;
			} else if (direction_what == "control_point") {
				r_ret = get_constraint(index)->get_direction(direction_index)->get_control_point();
				return true;
			}
		}
	}
	return false;
}

bool SkeletonModification3DDMIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;

	if (name == "root_bone") {
		set_root_bone(p_value);
		return true;
	} else if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, effector_count, false);
		Ref<BoneEffector> effector = get_effector(index);
		if (effector.is_null()) {
			effector.instance();
			set_effector(index, effector);
		}
		if (what == "name") {
			name = p_value;
			ERR_FAIL_COND_V(name.empty(), false);
			effector->set_name(name);
			emit_signal("ik_changed");
			return true;
		} else if (what == "target_node") {
			effector->set_target_node(p_value);
			emit_signal("ik_changed");
			return true;
		} else if (what == "target_transform") {
			effector->set_target_transform(p_value);
			emit_signal("ik_changed");
			return true;
		} else if (what == "budget") {
			effector->set_budget_ms(p_value);
			emit_signal("ik_changed");
			return true;
		}
	} else if (name.begins_with("kusudama_constraints/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, multi_constraint.size(), false);
		if (multi_constraint[index].is_null()) {
			multi_constraint.write[index].instance();
		}
		if (what == "name") {
			multi_constraint.write[index]->set_name(p_value);
			emit_signal("ik_changed");
			return true;
		} else if (what == "twist_min_angle") {
			Ref<KusudamaConstraint> constraint = get_constraint(index);
			if (constraint.is_null()) {
				constraint.instance();
			}
			Ref<TwistConstraint> twist = constraint->get_twist_constraint();
			twist->set_min_twist_angle(p_value);
			constraint->set_twist_constraint(twist);
			emit_signal("ik_changed");
			return true;
		} else if (what == "twist_range") {
			Ref<KusudamaConstraint> constraint = get_constraint(index);
			if (constraint.is_null()) {
				constraint.instance();
			}
			Ref<TwistConstraint> twist = constraint->get_twist_constraint();
			twist->set_range(p_value);
			constraint->set_twist_constraint(twist);
			emit_signal("ik_changed");
			return true;
		} else if (what == "constraint_axes") {
			Ref<KusudamaConstraint> constraint = multi_constraint[index];
			if (constraint.is_null()) {
				constraint.instance();
			}
			constraint->set_constraint_axes(p_value);
			_change_notify();
			emit_changed();
			emit_signal("ik_changed");
			return true;
		} else if (what == "direction_count") {
			Ref<KusudamaConstraint> constraint = multi_constraint[index];
			if (constraint.is_null()) {
				constraint.instance();
			}
			constraint->set_direction_count(p_value);
			_change_notify();
			emit_changed();
			emit_signal("ik_changed");
			return true;
		} else if (what == "direction") {
			int direction_index = name.get_slicec('/', 3).to_int();
			ERR_FAIL_INDEX_V(direction_index, multi_constraint[index]->get_direction_count(), false);
			ERR_FAIL_COND_V(multi_constraint[index]->get_direction(direction_index).is_null(), false);
			String direction_what = name.get_slicec('/', 4);
			if (direction_what == "radius") {
				Ref<DirectionConstraint> constraint = multi_constraint[index]->get_direction(direction_index);
				constraint->set_radius(p_value);
			} else if (direction_what == "control_point") {
				Ref<DirectionConstraint> constraint = multi_constraint[index]->get_direction(direction_index);
				constraint->set_control_point(p_value);
			} else {
				return false;
			}
			_change_notify();
			emit_changed();
			emit_signal("ik_changed");
			return true;
		}
	}
	return false;
}

Ref<KusudamaConstraint> SkeletonModification3DDMIK::get_constraint(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_constraint.size(), Ref<KusudamaConstraint>());
	ERR_FAIL_COND_V(multi_constraint[p_index].is_null(), Ref<KusudamaConstraint>());
	return multi_constraint[p_index];
}

SkeletonModification3DDMIK::SkeletonModification3DDMIK() {
}

SkeletonModification3DDMIK::~SkeletonModification3DDMIK() {
}

int32_t SkeletonModification3DDMIK::find_constraint(String p_name) {
	for (int32_t constraint_i = 0; constraint_i < get_constraint_count(); constraint_i++) {
		Ref<KusudamaConstraint> constraint = get_constraint(constraint_i);
		if (constraint.is_null()) {
			continue;
		}
		if (constraint->get_name() == p_name) {
			return constraint_i;
		}
	}
	return -1;
}

void SkeletonModification3DDMIK::set_effector_count(int32_t p_value) {
	multi_effector.resize(p_value);
	effector_count = p_value;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

int32_t SkeletonModification3DDMIK::get_effector_count() const {
	return effector_count;
}

Ref<BoneEffector> SkeletonModification3DDMIK::get_effector(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_effector.size(), NULL);
	Ref<BoneEffector> effector = multi_effector[p_index];
	return effector;
}

void SkeletonModification3DDMIK::set_effector(int32_t p_index, Ref<BoneEffector> p_effector) {
	ERR_FAIL_COND(p_effector.is_null());
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.write[p_index] = p_effector;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

void SkeletonModification3DDMIK::set_constraint(int32_t p_index, Ref<KusudamaConstraint> p_constraint) {
	ERR_FAIL_INDEX(p_index, multi_constraint.size());
	multi_constraint.write[p_index] = p_constraint;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

Vector<Ref<BoneEffector>> SkeletonModification3DDMIK::get_bone_effectors() const {
	return multi_effector;
}

int32_t SkeletonModification3DDMIK::find_effector(String p_name) {
	for (int32_t effector_i = 0; effector_i < multi_effector.size(); effector_i++) {
		if (multi_effector[effector_i].is_valid() && multi_effector[effector_i]->get_name() == p_name) {
			return effector_i;
		}
	}
	return -1;
}

void SkeletonModification3DDMIK::remove_effector(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.remove(p_index);
	effector_count--;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

int32_t SkeletonModification3DDMIK::get_constraint_count() const {
	return constraint_count;
}

void SkeletonModification3DDMIK::set_constraint_count(int32_t p_value) {
	multi_constraint.resize(p_value);
	for (int32_t i = 0; i < p_value; i++) {
		multi_constraint.write[i].instance();
	}
	constraint_count = p_value;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

void SkeletonModification3DDMIK::execute(float delta) {
	ERR_FAIL_COND_MSG(!stack || !is_setup || stack->skeleton == nullptr,
			"Modification is not setup and therefore cannot execute!");
	if (!enabled) {
		return;
	}
	task.instance();
	Skeleton3D *skeleton = stack->skeleton;
	task->skeleton = skeleton;
	if (!multi_effector.size()) {
		return;
	}
	String root_bone_name = get_root_bone();
	task->root_bone = skeleton->find_bone(root_bone_name);
	task->end_effectors.resize(multi_effector.size());
	for (int32_t effector_i = 0; effector_i < multi_effector.size(); effector_i++) {
		Transform xform;
		Ref<BoneEffector> bone_effector = multi_effector[effector_i];
		if (bone_effector.is_null()) {
			continue;
		}
		String bone_name = bone_effector->get_name();
		if (bone_name.empty()) {
			continue;
		}
		int32_t bone = skeleton->find_bone(bone_name);
		if (bone == -1) {
			continue;
		}
		Ref<BoneEffectorTransform> bone_end_effector;
		bone_end_effector.instance();
		bone_end_effector->effector_bone = bone;
		NodePath path = bone_effector->get_target_node();
		// TODO Add an path to object_id cache.
		Node *node = skeleton->get_node(path);
		if (node) {
			ERR_FAIL_COND_MSG(!node || skeleton == node,
					"Cannot update cache: Target node is this modification's skeleton or cannot be found!");
			Node3D *node_3d = Object::cast_to<Node3D>(node);
			if (node_3d) {
				xform = skeleton->world_transform_to_global_pose(node_3d->get_global_transform());
			}
		}
		bone_end_effector->goal_transform = xform * bone_effector->get_target_transform();
		task->end_effectors.write[effector_i] = bone_end_effector;
	}
	build_chain(task);
	if (task.is_valid()) {
		solve_simple(task, false);
	}
}

void SkeletonModification3DDMIK::setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;
	if (!stack) {
		return;
	}
	Skeleton3D *skeleton = stack->get_skeleton();
	ERR_FAIL_COND(!skeleton);
	if (!constraint_count) {
		register_constraint(skeleton);
	}
	register_effectors(skeleton);
	is_setup = true;
}

void SkeletonModification3DDMIK::add_effector(String p_name, NodePath p_node, Transform p_transform, real_t p_budget) {
	Ref<BoneEffector> effector;
	effector.instance();
	effector->set_name(p_name);
	effector->set_target_node(p_node);
	effector->set_target_transform(p_transform);
	effector->set_budget_ms(p_budget);
	multi_effector.push_back(effector);
	effector_count++;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

void SkeletonModification3DDMIK::register_constraint(Skeleton3D *p_skeleton) {
	ERR_FAIL_COND(!p_skeleton);
	for (int32_t bone_i = 0; bone_i < p_skeleton->get_bone_count(); bone_i++) {
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		String bone_name = p_skeleton->get_bone_name(bone_i);
		constraint->set_name(bone_name);
		multi_constraint.push_back(constraint);
		constraint_count++;
	}
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

/*************************************************************************/
/*  cmdd_inverse_kinematics.cpp                                          */
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

#include "scene/3d/skeleton_3d.h"

void SkeletonModification3DDMIK::QCPSolver(
		Skeleton3D *p_skeleton,
		Ref<BoneChain> p_chain,
		float p_dampening,
		bool p_inverse_weighting,
		int p_stabilization_passes,
		int p_iteration,
		float p_total_iterations) {
	for (int32_t tip_i = 0; tip_i < p_chain->targets.size(); tip_i++) {
		Ref<BoneChainItem> start_from = p_chain->targets[tip_i]->chain_item;
		Ref<BoneChainItem> stop_after = p_chain->chain_root;

		Ref<BoneChainItem> current_bone = start_from;
		//if the tip is pinned, it should have already been oriented before this function was called.
		while (current_bone.is_valid() && current_bone != stop_after) {
			if (!current_bone->ik_orientation_lock) {
				update_optimal_rotation_to_target_descendants(p_skeleton, p_chain, current_bone, p_dampening, false,
						p_stabilization_passes, p_iteration, p_total_iterations);
			}
			current_bone = current_bone->parent_item;
		}
	}
}

Ref<BoneChainItem> BoneChainItem::find_child(const int p_bone_id) {
	for (int i = children.size() - 1; 0 <= i; --i) {
		if (children[i].is_null()) {
			continue;
		}
		if (p_bone_id == children[i]->bone) {
			return children.write[i];
		}
	}
	return nullptr;
}

Ref<BoneChainItem> BoneChainItem::add_child(const int p_bone_id) {
	const int infant_child_id = children.size();
	children.resize(infant_child_id + 1);
	if (children.write[infant_child_id].is_null()) {
		children.write[infant_child_id].instance();
	}
	children.write[infant_child_id]->bone = p_bone_id;
	children.write[infant_child_id]->parent_item = Ref<BoneChainItem>(this);
	return children.write[infant_child_id];
}

void BoneChainItem::update_cos_dampening() {
	float predampening = 1.0f - get_stiffness();
	float default_dampening = parent_armature->dampening;
	float dampening = parent_item == NULL ? Math_PI : predampening * default_dampening;
	cos_half_dampen = Math::cos(dampening / 2.0f);
	Ref<KusudamaConstraint> k = constraint;
	if (k.is_valid() && k->get_pain() != 0.0f) {
		springy = true;
		populate_return_dampening_iteration_array(k);
	} else {
		springy = false;
	}
}

float BoneChainItem::get_stiffness() const {
	return stiffness_scalar;
}

void BoneChainItem::set_axes_to_returned(IKAxes p_global, IKAxes p_to_set, IKAxes p_limiting_axes,
		float p_cos_half_angle_dampen, float p_angle_dampen) {
	if (constraint.is_valid()) {
		constraint->set_axes_to_returnful(p_global, p_to_set, p_limiting_axes, p_cos_half_angle_dampen,
				p_angle_dampen);
	}
}

void BoneChainItem::set_axes_to_be_snapped(IKAxes p_to_set, IKAxes p_limiting_axes,
		float p_cos_half_angle_dampen) {
	if (constraint.is_valid()) {
		constraint->set_axes_to_snapped(p_to_set, p_limiting_axes, p_cos_half_angle_dampen);
	}
}

void BoneChainItem::set_stiffness(float p_stiffness) {
	stiffness_scalar = p_stiffness;
	if (parent_item.is_valid()) {
		parent_item->update_cos_dampening();
	}
}

/// Build a chain that starts from the root to tip
bool SkeletonModification3DDMIK::build_chain(Ref<DMIKTask> p_task) {
	ERR_FAIL_COND_V(-1 == p_task->root_bone, false);

	Ref<BoneChain> chain = p_task->chain;
	chain->targets.resize(p_task->end_effectors.size());
	chain->chain_root->bone = p_task->root_bone;
	chain->chain_root->axes = p_task->skeleton->get_bone_rest(chain->chain_root->bone) * p_task->skeleton->get_bone_pose(chain->chain_root->bone);
	chain->chain_root->pb = p_task->skeleton->get_physical_bone(chain->chain_root->bone);
	chain->middle_chain_item = Ref<BoneChainItem>();

	// Holds all IDs that are composing a single chain in reverse order
	Vector<int> chain_ids;
	// This is used to know the chain size
	// int sub_chain_size = 0;
	// Resize only one time in order to fit all joints for performance reason
	chain_ids.resize(p_task->skeleton->get_bone_count());

	for (int effector_i = p_task->end_effectors.size() - 1; 0 <= effector_i; --effector_i) {
		const Ref<BoneEffectorTransform> ee(p_task->end_effectors[effector_i]);
		ERR_FAIL_COND_V(p_task->root_bone >= ee->effector_bone, false);
		ERR_FAIL_INDEX_V(ee->effector_bone, p_task->skeleton->get_bone_count(), false);

		int32_t current_chain_size = 0;
		// Picks all IDs that composing a single chain in reverse order (except the root)
		int chain_sub_tip(ee->effector_bone);
		while (chain_sub_tip > p_task->root_bone) {
			chain_ids.write[current_chain_size++] = chain_sub_tip;
			chain_sub_tip = p_task->skeleton->get_bone_parent(chain_sub_tip);
		}

		int middle_chain_item_id = (((float)current_chain_size) * 0.5);

		// Build the chain by reading the chain ids in reverse order
		// For each chain item an id will be created if it doesn't exists
		Ref<BoneChainItem> sub_chain(chain->chain_root);
		for (int chain_i = current_chain_size - 1; 0 <= chain_i; --chain_i) {
			Ref<BoneChainItem> child_ci(sub_chain->find_child(chain_ids[chain_i]));
			if (child_ci.is_null()) {
				child_ci = sub_chain->add_child(chain_ids[chain_i]);

				child_ci->pb = p_task->skeleton->get_physical_bone(child_ci->bone);
				//if (child_ci->parent_item.is_valid()) {
				//	child_ci->axes->parent = child_ci->parent_item->axes;
				//	child_ci->parent_item->children.push_back(child_ci->axes);
				//}
				child_ci->axes = p_task->skeleton->get_bone_rest(child_ci->bone) * p_task->skeleton->get_bone_pose(child_ci->bone);

				if (child_ci->parent_item.is_valid()) {
					child_ci->length = (child_ci->axes.origin - child_ci->parent_item->axes.origin).length();
				}
			}

			sub_chain = child_ci;

			if (middle_chain_item_id == chain_i) {
				chain->middle_chain_item = child_ci;
			}
		}
		if (!middle_chain_item_id) {
			chain->middle_chain_item = Ref<BoneChainItem>();
		}

		if (chain->targets.write[effector_i].is_null()) {
			chain->targets.write[effector_i].instance();
		}

		// Initialize current tip
		chain->targets.write[effector_i]->chain_item = sub_chain;
		chain->targets.write[effector_i]->end_effector = ee;
	}
	chain->create_headings_arrays();

	return true;
}

void SkeletonModification3DDMIK::update_chain(const Skeleton3D *p_sk, Ref<BoneChainItem> p_chain_item) {
	if (p_chain_item.is_null()) {
		return;
	}
	p_chain_item->axes =
			p_sk->get_bone_rest(p_chain_item->bone) * p_sk->get_bone_pose(p_chain_item->bone);
	if (!p_chain_item->children.size()) {
		return;
	}
	for (int i = p_chain_item->children.size() - 1; 0 <= i; --i) {
		update_chain(p_sk, p_chain_item->children.write[i]);
	}
}

void SkeletonModification3DDMIK::solve_simple(Ref<DMIKTask> p_task, bool p_solve_magnet) {
	QCPSolver(
			p_task->skeleton,
			p_task->chain,
			p_task->dampening,
			false,
			p_task->iterations,
			p_task->stabilizing_passes,
			p_task->max_iterations);
}

Ref<DMIKTask> SkeletonModification3DDMIK::create_simple_task(Skeleton3D *p_sk,
		const Transform &goal_transform,
		float p_dampening, int p_stabilizing_passes,
		Ref<SkeletonModification3DDMIK> p_constraints) {
	Ref<DMIKTask> task;
	task.instance();
	task->skeleton = p_sk;

	for (int32_t root_i = 0; root_i < p_sk->get_bone_count(); root_i++) {
		if (p_sk->get_bone_parent(root_i) != -1) {
			continue;
		}
		task->root_bone = root_i;
		break;
	}

	ERR_FAIL_COND_V(task->root_bone == -1, NULL);
	ERR_FAIL_COND_V(p_constraints.is_null(), NULL);

	{
		Ref<KusudamaConstraint> constraint;
		constraint.instance();
		constraint->set_name(p_sk->get_bone_name(task->root_bone));
		if (!p_constraints->get_constraint_count()) {
			p_constraints->set_constraint_count(1);
		}
		p_constraints->set_constraint(0, constraint);
	}
	for (int32_t name_i = 0; name_i < p_constraints->get_effector_count(); name_i++) {
		Ref<BoneEffector> effector = p_constraints->get_effector(name_i);
		if (effector.is_null()) {
			continue;
		}
		Ref<BoneEffectorTransform> ee;
		ee.instance();
		Node *target_node = task->skeleton->get_node_or_null(effector->get_target_node());
		Node3D *spatial_node = Object::cast_to<Node3D>(target_node);
		Transform xform = effector->get_target_transform();
		if (spatial_node) {
			xform *= task->skeleton->get_global_transform().affine_inverse() * spatial_node->get_global_transform();
		}
		int32_t bone = task->skeleton->find_bone(effector->get_name());
		if (bone == -1) {
			continue;
		}
		ee->goal_transform *= xform;
		ee->effector_bone = bone;
		task->end_effectors.push_back(ee);
	}

	for (int32_t constraint_i = 0; constraint_i < p_constraints->get_constraint_count(); constraint_i++) {
		Ref<KusudamaConstraint> constraint = p_constraints->get_constraint(constraint_i);
		ERR_CONTINUE(constraint.is_null());
		for (int32_t direction_i = 0; direction_i < constraint->get_direction_count(); direction_i++) {
			Ref<DirectionConstraint> direction = constraint->get_direction(direction_i);
			if (direction.is_null()) {
				continue;
			}
			Vector3 cp = direction->get_control_point();
			direction->set_control_point(cp.normalized());
			constraint->set_direction(direction_i, direction);
		}
	}

	task->dampening = p_dampening;
	task->stabilizing_passes = p_stabilizing_passes;
	if (!build_chain(task)) {
		return NULL;
	}

	return task;
}

void SkeletonModification3DDMIK::make_goal(Ref<DMIKTask> p_task, const Transform &p_inverse_transf, float blending_delta) {
	if (!p_task->end_effectors.size()) {
		return;
	}
	if (blending_delta >= 0.99f) {
		// Update the end_effector (local transform) without blending
		p_task->end_effectors.write[0]->goal_transform = p_inverse_transf * p_task->end_effectors.write[0]->goal_transform;
	} else {
		// End effector in local transform
		const Transform end_effector_pose(
				p_task->skeleton->get_bone_rest(p_task->end_effectors.write[0]->effector_bone) * p_task->skeleton->get_bone_pose(p_task->end_effectors.write[0]->effector_bone));
		print_verbose("IK make goal bone " + p_task->skeleton->get_bone_name(p_task->end_effectors.write[0]->effector_bone));
		print_verbose("IK make goal bone local location " + end_effector_pose.origin);
		print_verbose("IK make goal bone global location " + p_task->skeleton->get_bone_global_pose(p_task->end_effectors.write[0]->effector_bone).origin);
		// Update the end_effector (local transform) by blending with current pose
		p_task->end_effectors.write[0]
				->goal_transform = end_effector_pose.interpolate_with(
				p_inverse_transf * p_task->end_effectors[0]->goal_transform, blending_delta);
	}
}

void SkeletonModification3DDMIK::solve(Ref<DMIKTask> p_task, float blending_delta, bool override_tip_basis, bool p_use_magnet,
		const Vector3 &p_magnet_position) {
	if (blending_delta <= 0.01f) {
		return; // Skip solving
	}

	make_goal(p_task, p_task->skeleton->get_global_transform().affine_inverse().scaled(p_task->skeleton->get_global_transform().get_basis().get_scale()), blending_delta);

	update_chain(p_task->skeleton, p_task->chain->chain_root);

	if (!p_task->end_effectors.size()) {
		return;
	}

	//print_verbose("IK solve bone " + p_task->skeleton->get_bone_name(p_task->end_effectors[0]->effector_bone));
	//print_verbose("IK solve bone goal local location " + p_task->end_effectors[0]->goal_transform.origin);
	Ref<BoneChainItem> effector_chain_item = p_task->chain->chain_root->find_child(p_task->end_effectors[0]->effector_bone);
	//if (effector_chain_item.is_valid()) {
	//	print_verbose("IK solve bone local location " + effector_chain_item->axes->local_transform.origin);
	//	print_verbose("IK solve bone global location " + p_task->skeleton->get_bone_global_pose(effector_chain_item->bone).origin);
	//}

	solve_simple(p_task, false);

	// Assign new bone position.
	Ref<BoneChainItem> ci(p_task->chain->chain_root);
	while (ci.is_valid()) {
		Transform new_bone_pose(p_task->skeleton->get_bone_global_pose(ci->bone));
		new_bone_pose.origin = ci->axes.origin;

		if (!ci->children.empty()) {
			Vector3 child_pose = p_task->skeleton->get_bone_global_pose(ci->children[0]->bone).origin;
			Vector3 current_pose = p_task->skeleton->get_bone_global_pose(ci->bone).origin;
			const Vector3 initial_ori = (child_pose - current_pose).normalized();
			const Vector3 rot_axis = (initial_ori.cross(ci->axes.get_basis().get_rotation_euler()).normalized());

			if (rot_axis[0] != 0 && rot_axis[1] != 0 && rot_axis[2] != 0) {
				const float rot_angle(Math::acos(CLAMP(initial_ori.dot(ci->axes.get_basis().get_rotation_euler()), -1, 1)));
				new_bone_pose.basis.rotate(rot_axis, rot_angle);
			}
		} else {
			// Set target orientation to tip
			if (override_tip_basis)
				new_bone_pose.basis = p_task->chain->targets[0]->end_effector->goal_transform.basis;
			else
				new_bone_pose.basis =
						new_bone_pose.basis * p_task->chain->targets[0]->end_effector->goal_transform.basis;
		}

		p_task->skeleton->set_bone_global_pose_override(ci->bone, new_bone_pose, 1.0);

		if (!ci->children.empty())
			ci = ci->children.write[0];
		else
			ci = Ref<BoneChainItem>();
	}
}

void SkeletonModification3DDMIK::set_default_dampening(Ref<BoneChain> r_chain, float p_damp) {
	r_chain->dampening =
			MIN(Math_PI * 3.0f, MAX(Math::absf(std::numeric_limits<real_t>::epsilon()), Math::absf(p_damp)));
	update_armature_segments(r_chain);
}

void SkeletonModification3DDMIK::update_armature_segments(Ref<BoneChain> r_chain) {
	r_chain->bone_segment_map.clear();
	recursively_update_bone_segment_map_from(r_chain, r_chain->chain_root);
}

void SkeletonModification3DDMIK::update_optimal_rotation_to_target_descendants(Skeleton3D *p_skeleton, Ref<BoneChainItem> p_chain_item,
		float p_dampening, bool p_is_translate,
		Vector<Vector3> p_localized_tip_headings,
		Vector<Vector3> p_localized_target_headings,
		Vector<real_t> p_weights,
		Ref<QCP> p_qcp_orientation_aligner, int p_iteration,
		float p_total_iterations) {
	p_qcp_orientation_aligner->set_max_iterations(10);
	IKQuat qcp_rot = p_qcp_orientation_aligner->weighted_superpose(p_localized_tip_headings, p_localized_target_headings,
			p_weights, p_is_translate);

	Vector3 translate_by = p_qcp_orientation_aligner->get_translation();
	float bone_damp = p_chain_item->cos_half_dampen;

	if (p_dampening != -1) {
		bone_damp = p_dampening;
		qcp_rot.clamp_to_angle(bone_damp);
	} else {
		qcp_rot.clamp_to_quadrance_angle(bone_damp);
	}
	p_chain_item->axes.origin *= translate_by;
	p_chain_item->axes.basis *= Basis(qcp_rot);
	IKAxes xform;
	xform.basis = p_chain_item->axes.get_basis();
	if (p_chain_item->constraint.is_null()) {
		return;
	}
	p_chain_item->set_axes_to_be_snapped(xform, p_chain_item->constraint->get_limiting_axes(), bone_damp);
	xform.origin = p_chain_item->axes.origin;
	p_chain_item->constraint->set_limiting_axes(p_chain_item->constraint->get_limiting_axes().translated(translate_by));
}

void SkeletonModification3DDMIK::recursively_update_bone_segment_map_from(Ref<BoneChain> r_chain,
		Ref<BoneChainItem> p_start_from) {
	for (int32_t child_i = 0; child_i < p_start_from->children.size(); child_i++) {
		r_chain->bone_segment_map.insert(p_start_from->children[child_i]->bone, p_start_from);
	}
}

void SkeletonModification3DDMIK::update_optimal_rotation_to_target_descendants(Skeleton3D *p_skeleton,
		Ref<BoneChain> r_chain,
		Ref<BoneChainItem> p_for_bone,
		float p_dampening, bool p_translate,
		int p_stabilization_passes, int p_iteration,
		int p_total_iterations) {
	if (p_for_bone.is_null()) {
		return;
	}

	Quat best_orientation = p_for_bone->axes.get_basis().get_rotation_quat();
	float new_dampening = -1;
	if (p_for_bone->parent_item == NULL)
		p_stabilization_passes = 0;
	if (p_translate == true) {
		new_dampening = Math_PI;
	}
	IKAxes bone_xform;
	Quat quat = p_for_bone->axes.basis.get_rotation_quat();
	bone_xform.basis = Basis(quat);
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
				if (p_for_bone->springy) {
					if (p_dampening != -1 || p_total_iterations != r_chain->get_default_iterations()) {
						float returnfullness = p_for_bone->constraint->get_pain();
						float dampened_angle = p_for_bone->get_stiffness() * p_dampening * returnfullness;
						float total_iterations_sq = p_total_iterations * p_total_iterations;
						float scaled_dampened_angle = dampened_angle *
													  ((total_iterations_sq - (p_iteration * p_iteration)) /
															  total_iterations_sq);
						float cos_half_angle = Math::cos(0.5f * scaled_dampened_angle);
						p_for_bone->set_axes_to_returned(p_skeleton->get_bone_global_pose(p_for_bone->bone), bone_xform,
								p_for_bone->constraint->get_limiting_axes(), cos_half_angle,
								scaled_dampened_angle);
					} else {
						p_for_bone->set_axes_to_returned(p_skeleton->get_bone_global_pose(p_for_bone->bone), bone_xform,
								p_for_bone->constraint->get_limiting_axes(),
								p_for_bone->cos_half_returnful_dampened[p_iteration],
								p_for_bone->half_returnful_dampened[p_iteration]);
					}
					update_effector_headings(r_chain, r_chain->localized_effector_headings, bone_xform);
					new_rmsd = get_manual_msd(r_chain->localized_effector_headings, r_chain->localized_target_headings,
							r_chain->weights);
				}
				best_orientation = bone_xform.basis.get_rotation_quat();
				best_rmsd = new_rmsd;
				break;
			}
		} else {
			break;
		}
	}
	if (p_stabilization_passes > 0) {
		bone_xform.basis = Basis(best_orientation);
		p_for_bone->axes.origin = bone_xform.origin;
		p_for_bone->axes.basis = bone_xform.basis;
	}
}

float SkeletonModification3DDMIK::get_manual_msd(Vector<Vector3> &r_localized_effector_headings,
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

void SkeletonModification3DDMIK::update_target_headings(Ref<BoneChain> r_chain, Vector<Vector3> &r_localized_target_headings,
		Vector<real_t> p_weights, Transform p_bone_xform) {
	int hdx = 0;
	for (int target_i = 0; target_i < r_chain->targets.size(); target_i++) {
		Ref<BoneChainItem> sb = r_chain->targets[target_i]->chain_item;
		if (sb->constraint.is_null()) {
			continue;
		}
		IKAxes targetAxes = sb->constraint->get_limiting_axes();
		Vector3 origin = sb->axes.origin;
		r_localized_target_headings.write[hdx] = targetAxes.origin - origin;
		uint8_t modeCode = r_chain->targets[target_i]->get_mode_code();
		hdx++;
		if ((modeCode & BoneChainTarget::XDir) != 0) {
			Ray xTarget;
			xTarget.normal = Vector3(1, 0, 0);
			xTarget.position = xTarget.normal * targetAxes.basis.get_axis(x_axis);
			r_localized_target_headings.write[hdx] = xTarget.position - origin;
			// xTarget.position = xTarget.setToInvertedTip(r_localized_target_headings[hdx + 1]) - origin;
			hdx += 2;
		}
		if ((modeCode & BoneChainTarget::YDir) != 0) {
			Ray yTarget;
			yTarget.normal = Vector3(0, 1, 0);
			yTarget.position = yTarget.normal * targetAxes.basis.get_axis(y_axis);
			r_localized_target_headings.write[hdx] = yTarget.position - origin;
			// yTarget.position = yTarget.setToInvertedTip(r_localized_target_headings[hdx + 1]) - origin;
			hdx += 2;
		}
		if ((modeCode & BoneChainTarget::ZDir) != 0) {
			Ray zTarget;
			zTarget.normal = Vector3(0, 0, 1);
			zTarget.position = zTarget.normal * targetAxes.basis.get_axis(z_axis);
			r_localized_target_headings.write[hdx] = zTarget.position - origin;
			// zTarget.position = zTarget.setToInvertedTip(r_localized_target_headings[hdx + 1]) - origin;
			hdx += 2;
		}
	}
}

void SkeletonModification3DDMIK::update_effector_headings(Ref<BoneChain> r_chain, Vector<Vector3> &r_localized_effector_headings,
		Transform p_bone_xform) {
	int hdx = 0;
	for (int target_i = 0; target_i < r_chain->targets.size(); target_i++) {
		Ref<BoneChainItem> sb = r_chain->targets[target_i]->chain_item;
		IKAxes effector = r_chain->targets[target_i]->end_effector->goal_transform;
		Vector3 origin = sb->axes.origin;
		r_localized_effector_headings.write[hdx] = effector.origin - origin;
		uint8_t modeCode = r_chain->targets[target_i]->get_mode_code();
		hdx++;

		if ((modeCode & BoneChainTarget::XDir) != 0) {
			Ray xEffector;
			xEffector.normal = Vector3(1, 0, 0);
			xEffector.position = xEffector.normal * effector.basis.get_axis(x_axis);

			r_localized_effector_headings.write[hdx] = xEffector.position - origin;
			// xTip.setToInvertedTip(r_localized_effector_headings[hdx+1]).sub(origin);
			hdx += 2;
		}
		if ((modeCode & BoneChainTarget::YDir) != 0) {
			Ray yEffector;

			yEffector.normal = Vector3(0, 1, 0);
			yEffector.position = yEffector.normal * effector.basis.get_axis(y_axis);

			r_localized_effector_headings.write[hdx] = yEffector.position - origin;
			// yEffector.setToInvertedTip(r_localized_effector_headings[hdx+1]).sub(origin);
			hdx += 2;
		}
		if ((modeCode & BoneChainTarget::ZDir) != 0) {
			Ray zEffector;

			zEffector.normal = Vector3(0, 0, 1);
			zEffector.position = zEffector.normal * effector.basis.get_axis(z_axis);

			r_localized_effector_headings.write[hdx] = zEffector.position - origin;
			// zEffector.setToInvertedTip(r_localized_effector_headings[hdx+1]).sub(origin);
			hdx += 2;
		}
	}
}

int BoneChain::get_default_iterations() const {
	return ik_iterations;
}

void BoneChainTarget::set_target_priorities(float p_x_priority, float p_y_priority, float p_z_priority) {
	bool x_dir = p_x_priority > 0 ? true : false;
	bool y_dir = p_y_priority > 0 ? true : false;
	bool z_dir = p_z_priority > 0 ? true : false;
	mode_code = 0;
	if (x_dir)
		mode_code += XDir;
	if (y_dir)
		mode_code += YDir;
	if (z_dir)
		mode_code += ZDir;

	sub_target_count = 1;
	if ((mode_code & 1) != 0)
		sub_target_count++;
	if ((mode_code & 2) != 0)
		sub_target_count++;
	if ((mode_code & 4) != 0)
		sub_target_count++;

	x_priority = p_x_priority;
	y_priority = p_y_priority;
	z_priority = p_z_priority;
	chain_item->parent_item->rootwardly_update_falloff_cache_from(for_bone());
}

float BoneChainTarget::get_depth_falloff() const {
	return depthFalloff;
}

void BoneChainTarget::set_depth_falloff(float depth) {
	depthFalloff = depth;
	chain_item->parent_item->rootwardly_update_falloff_cache_from(for_bone());
}

void BoneChainTarget::disable() {
	enabled = false;
}

void BoneChainTarget::enable() {
	enabled = true;
}

void BoneChainTarget::toggle() {
	if (is_enabled()) {
		disable();
	} else {
		enable();
	}
}

bool BoneChainTarget::is_enabled() const {
	return enabled;
}

int BoneChainTarget::get_subtarget_count() {
	return sub_target_count;
}

uint8_t BoneChainTarget::get_mode_code() const {
	return mode_code;
}

float BoneChainTarget::get_x_priority() const {
	return x_priority;
}

float BoneChainTarget::get_y_priority() const {
	return y_priority;
}

float BoneChainTarget::get_z_priority() const {
	return z_priority;
}

IKAxes BoneChainTarget::get_axes() const {
	return chain_item->axes;
}

void BoneChainTarget::align_to_axes(IKAxes inAxes) {
	//TODO
	//axes.alignGlobalsTo(inAxes);
}

void BoneChainTarget::translate(Vector3 location) {
	chain_item->axes.origin *= location;
}

Vector3 BoneChainTarget::get_location() {
	return chain_item->axes.origin;
}
Ref<BoneChainItem> BoneChainTarget::for_bone() {
	return chain_item;
}

void BoneChainTarget::removal_notification() {
	for (int32_t target_i = 0; target_i < child_targets.size(); target_i++) {
		child_targets.write[target_i]->set_parent_target(get_parent_target());
	}
}

void BoneChainItem::rootwardly_update_falloff_cache_from(Ref<BoneChainItem> p_current) {
	Ref<BoneChainItem> current = p_current;
	while (current.is_valid()) {
		current->parent_armature->create_headings_arrays();
		current = current->parent_item;
	}
}

void BoneChain::recursively_create_penalty_array(Ref<BoneChain> from,
		Vector<Vector<real_t>> &r_weight_array,
		Vector<Ref<BoneChainItem>>

				pin_sequence,
		float current_falloff) {
	if (current_falloff == 0) {
		return;
	} else {
		for (int32_t target_i = 0; target_i < from->targets.size(); target_i++) {
			Ref<BoneChainTarget> target = from->targets[target_i];
			if (target.is_valid()) {
				Vector<real_t> inner_weight_array;
				uint8_t mode_code = target->get_mode_code();
				inner_weight_array.push_back(target->get_target_weight() * current_falloff);
				float max_target_weight = 0.0f;
				if ((mode_code & BoneChainTarget::XDir) != 0)
					max_target_weight = MAX(max_target_weight, target->get_x_priority());
				if ((mode_code & BoneChainTarget::YDir) != 0)
					max_target_weight = MAX(max_target_weight, target->get_y_priority());
				if ((mode_code & BoneChainTarget::ZDir) != 0)
					max_target_weight = MAX(max_target_weight, target->get_z_priority());

				if (max_target_weight == 0.0f)
					max_target_weight = 1.0f;

				max_target_weight = 1.0f;

				if ((mode_code & BoneChainTarget::XDir) != 0) {
					float sub_target_weight = target->get_target_weight() * (target->get_x_priority() / max_target_weight) * current_falloff;
					inner_weight_array.push_back(sub_target_weight);
					inner_weight_array.push_back(sub_target_weight);
				}
				if ((mode_code & BoneChainTarget::YDir) != 0) {
					float sub_target_weight = target->get_target_weight() * (target->get_y_priority() / max_target_weight) * current_falloff;
					inner_weight_array.push_back(sub_target_weight);
					inner_weight_array.push_back(sub_target_weight);
				}
				if ((mode_code & BoneChainTarget::ZDir) != 0) {
					float sub_target_weight = target->get_target_weight() * (target->get_z_priority() / max_target_weight) * current_falloff;
					inner_weight_array.push_back(sub_target_weight);
					inner_weight_array.push_back(sub_target_weight);
				}
				pin_sequence.push_back(target->for_bone());
				r_weight_array.push_back(inner_weight_array);
			}
			float this_falloff = target.is_null() ? 1.0f : target->get_depth_falloff();
			recursively_create_penalty_array(this, r_weight_array, pin_sequence, current_falloff * this_falloff);
		}
	}
}

void BoneChain::create_headings_arrays() {
	Vector<Vector<real_t>> penalty_array;
	Vector<Ref<BoneChainItem>> target_sequence;
	recursively_create_penalty_array(this, penalty_array, target_sequence, 1.0f);
	Vector<Ref<BoneChainItem>> target_bones;
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

void BoneChainTarget::set_parent_target(BoneChainTarget *parent) {
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

void BoneChainTarget::remove_child_target(BoneChainTarget *child) {
	int32_t target_i = child_targets.find(child);
	if (target_i != -1) {
		child_targets.remove(target_i);
	}
}

void BoneChainTarget::add_child_target(BoneChainTarget *newChild) {
	if (newChild->is_ancestor_of(this)) {
		set_parent_target(newChild->get_parent_target());
	}
	if (child_targets.find(newChild) != -1) {
		child_targets.push_back(newChild);
	}
}

BoneChainTarget *BoneChainTarget::get_parent_target() {
	return parent_target;
}

bool BoneChainTarget::is_ancestor_of(BoneChainTarget *potentialDescendent) {
	bool result = false;
	BoneChainTarget *cursor = potentialDescendent->get_parent_target();
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

float BoneChainTarget::get_target_weight() {
	return target_weight;
}

void BoneChainItem::populate_return_dampening_iteration_array(Ref<KusudamaConstraint> k) {
	float predampen = 1.0f - get_stiffness();
	float default_dampening = parent_armature->dampening;
	float dampening = parent_item == NULL ? Math_PI : predampen * default_dampening;
	float iterations = parent_armature->get_default_iterations();
	float returnful = k->get_pain();
	float falloff = 0.2f;
	half_returnful_dampened.resize(iterations);
	cos_half_returnful_dampened.resize(iterations);
	float iterations_pow = Math::pow(iterations, falloff * iterations * returnful);
	for (int32_t iter_i = 0; iter_i < iterations; iter_i++) {
		float iteration_scalar =
				((iterations_pow)-Math::pow(iter_i, falloff * iterations * returnful)) / (iterations_pow);
		float iteration_return_clamp = iteration_scalar * returnful * dampening;
		float cos_iteration_return_clamp = Math::cos(iteration_return_clamp / 2.0f);
		half_returnful_dampened.write[iter_i] = iteration_return_clamp;
		cos_half_returnful_dampened.write[iter_i] = cos_iteration_return_clamp;
	}
}

float BoneChainItem::get_bone_height() const {
	return bone_height;
}

void BoneChainItem::set_bone_height(const float p_bone_height) {
	bone_height = p_bone_height;
}
