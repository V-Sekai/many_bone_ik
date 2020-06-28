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

#include "direction_constraint.h"
#include "kusudama_constraint.h"
#include "skeleton_modification_3d_dmik.h"
#include "scene/3d/skeleton_3d.h"

void SkeletonModification3D_DMIK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_constraint_count", "constraint_count"),
			&SkeletonModification3D_DMIK::set_constraint_count);
	ClassDB::bind_method(D_METHOD("get_constraint_count"), &SkeletonModification3D_DMIK::get_constraint_count);
	ClassDB::bind_method(D_METHOD("get_effector_count"), &SkeletonModification3D_DMIK::get_effector_count);
	ClassDB::bind_method(D_METHOD("set_effector_count", "count"),
			&SkeletonModification3D_DMIK::set_effector_count);
	ClassDB::bind_method(D_METHOD("add_effector", "name", "target_node", "target_transform", "budget"), &SkeletonModification3D_DMIK::add_effector);
	ClassDB::bind_method(D_METHOD("get_effector", "index"), &SkeletonModification3D_DMIK::get_effector);
	ClassDB::bind_method(D_METHOD("get_constraint", "index"), &SkeletonModification3D_DMIK::get_constraint);
	ClassDB::bind_method(D_METHOD("set_effector", "index", "effector"), &SkeletonModification3D_DMIK::set_effector);
	ClassDB::bind_method(D_METHOD("set_constraint", "index", "constraint"), &SkeletonModification3D_DMIK::set_constraint);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "constraint_count", PROPERTY_HINT_RANGE, "0,65535,or_greater"), "set_constraint_count", "get_constraint_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "effector_count", PROPERTY_HINT_RANGE, "0,65535,or_greater"), "set_effector_count", "get_effector_count");
	ADD_SIGNAL(MethodInfo("ik_changed"));
}

void SkeletonModification3D_DMIK::_get_property_list(List<PropertyInfo> *p_list) const {
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
}

bool SkeletonModification3D_DMIK::_get(const StringName &p_name, Variant &r_ret) const {

	String name = p_name;
	if (name.begins_with("effectors/")) {
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
			ERR_FAIL_COND_V(get_constraint(index)->get_direction(direction_index).is_null(), false);
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

bool SkeletonModification3D_DMIK::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name.begins_with("effectors/")) {
		int index = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		ERR_FAIL_INDEX_V(index, effector_count, false);
		Ref<BoneEffector> effector = get_effector(index);
		ERR_FAIL_COND_V(effector.is_null(), false);
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

Ref<KusudamaConstraint> SkeletonModification3D_DMIK::get_constraint(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_constraint.size(), Ref<KusudamaConstraint>());
	ERR_FAIL_COND_V(multi_constraint[p_index].is_null(), Ref<KusudamaConstraint>());
	return multi_constraint[p_index];
}

SkeletonModification3D_DMIK::SkeletonModification3D_DMIK() {
}

SkeletonModification3D_DMIK::~SkeletonModification3D_DMIK() {
}

int32_t SkeletonModification3D_DMIK::find_constraint(String p_name) {
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

void SkeletonModification3D_DMIK::set_effector_count(int32_t p_value) {
	multi_effector.resize(p_value);
	effector_count = p_value;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

int32_t SkeletonModification3D_DMIK::get_effector_count() const {
	return effector_count;
}

Ref<BoneEffector> SkeletonModification3D_DMIK::get_effector(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_effector.size(), NULL);
	Ref<BoneEffector> effector = multi_effector[p_index];
	return effector;
}

void SkeletonModification3D_DMIK::set_effector(int32_t p_index, Ref<BoneEffector> p_effector) {
	ERR_FAIL_COND(p_effector.is_null());
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.write[p_index] = p_effector;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

void SkeletonModification3D_DMIK::set_constraint(int32_t p_index, Ref<KusudamaConstraint> p_constraint) {

	ERR_FAIL_INDEX(p_index, multi_constraint.size());
	multi_constraint.write[p_index] = p_constraint;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

Vector<Ref<BoneEffector> > SkeletonModification3D_DMIK::get_bone_effectors() const {
	return multi_effector;
}

int32_t SkeletonModification3D_DMIK::find_effector(String p_name) {
	for (int32_t effector_i = 0; effector_i < multi_effector.size(); effector_i++) {
		if (multi_effector[effector_i].is_valid() && multi_effector[effector_i]->get_name() == p_name) {
			return effector_i;
		}
	}
	return -1;
}

void SkeletonModification3D_DMIK::remove_effector(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, multi_effector.size());
	multi_effector.remove(p_index);
	effector_count--;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

int32_t SkeletonModification3D_DMIK::get_constraint_count() const {
	return constraint_count;
}

void SkeletonModification3D_DMIK::set_constraint_count(int32_t p_value) {
	multi_constraint.resize(p_value);
	for (int32_t i = 0; i < p_value; i++) {
		multi_constraint.write[i].instance();
	}
	constraint_count = p_value;
	_change_notify();
	emit_changed();
	emit_signal("ik_changed");
}

void SkeletonModification3D_DMIK::add_effector(String p_name, NodePath p_node, Transform p_transform, real_t p_budget) {
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

void SkeletonModification3D_DMIK::register_constraint(Skeleton3D *p_skeleton) {
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
