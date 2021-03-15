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

void EWBIKBoneEffector3D::set_target_transform(Transform p_target_transform) {
	target_transform = p_target_transform;
}

Transform EWBIKBoneEffector3D::get_target_transform() const {
	return target_transform;
}

void EWBIKBoneEffector3D::set_target_node(NodePath p_target_node_path) {
	target_node = p_target_node_path;
}

NodePath EWBIKBoneEffector3D::get_target_node() const {
	return target_node;
}

void EWBIKBoneEffector3D::set_use_target_node_rotation(bool p_use) {
	use_target_node_rotation = p_use;
}

bool EWBIKBoneEffector3D::get_use_target_node_rotation() const {
	return use_target_node_rotation;
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
