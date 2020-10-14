/*************************************************************************/
/*  bone_effector.cpp                                                    */
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

#include "bone_effector.h"

void DMIKBoneEffector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_transform", "transform"),
			&DMIKBoneEffector::set_target_transform);
	ClassDB::bind_method(D_METHOD("get_target_transform"),
			&DMIKBoneEffector::get_target_transform);

	ClassDB::bind_method(D_METHOD("set_target_node", "node"),
			&DMIKBoneEffector::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"),
			&DMIKBoneEffector::get_target_node);

	ClassDB::bind_method(D_METHOD("set_budget", "ms"),
			&DMIKBoneEffector::set_budget_ms);
	ClassDB::bind_method(D_METHOD("get_budget"),
			&DMIKBoneEffector::get_budget_ms);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "target_transform"), "set_target_transform", "get_target_transform");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "budget"), "set_budget", "get_budget");
}

void DMIKBoneEffector::set_target_transform(Transform p_target_transform) {
	target_transform = p_target_transform;
}

Transform DMIKBoneEffector::get_target_transform() const {
	return target_transform;
}

void DMIKBoneEffector::set_target_node(NodePath p_target_node_path) {
	target_node = p_target_node_path;
}

NodePath DMIKBoneEffector::get_target_node() const {
	return target_node;
}

real_t DMIKBoneEffector::get_budget_ms() const {
	return budget_ms;
}

void DMIKBoneEffector::set_budget_ms(real_t p_budget_ms) {
	budget_ms = p_budget_ms;
}
