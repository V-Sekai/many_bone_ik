/*************************************************************************/
/*  ik_effector_template.cpp                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "many_bone_ik_3d.h"

#include "ik_effector_template.h"

void IKEffectorTemplate::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_target_node"), &IKEffectorTemplate::get_target_node);
	ClassDB::bind_method(D_METHOD("set_target_node", "target_node"), &IKEffectorTemplate::set_target_node);

	ClassDB::bind_method(D_METHOD("get_passthrough_factor"), &IKEffectorTemplate::get_passthrough_factor);
	ClassDB::bind_method(D_METHOD("set_passthrough_factor", "passthrough_factor"), &IKEffectorTemplate::set_passthrough_factor);

	ClassDB::bind_method(D_METHOD("get_weight"), &IKEffectorTemplate::get_weight);
	ClassDB::bind_method(D_METHOD("set_weight", "weight"), &IKEffectorTemplate::set_weight);

	ClassDB::bind_method(D_METHOD("get_direction_priorities"), &IKEffectorTemplate::get_direction_priorities);
	ClassDB::bind_method(D_METHOD("set_direction_priorities", "direction_priorities"), &IKEffectorTemplate::set_direction_priorities);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "passthrough_factor"), "set_passthrough_factor", "get_passthrough_factor");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "weight"), "set_weight", "get_weight");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "direction_priorities"), "set_direction_priorities", "get_direction_priorities");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_node"), "set_target_node", "get_target_node");
}

NodePath IKEffectorTemplate::get_target_node() const {
	return target_node;
}

void IKEffectorTemplate::set_target_node(NodePath p_node_path) {
	target_node = p_node_path;
}

float IKEffectorTemplate::get_passthrough_factor() const {
	return passthrough_factor;
}

void IKEffectorTemplate::set_passthrough_factor(float p_passthrough_factor) {
	passthrough_factor = p_passthrough_factor;
}

IKEffectorTemplate::IKEffectorTemplate() {
}
