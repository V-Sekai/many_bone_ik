/*************************************************************************/
/*  ik_transform.cpp                                                     */
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

#include "ik_transform.h"

void IKTransform3D::_propagate_transform_changed() {
	for (List<IKTransform3D *>::Element *E = children.front(); E; E = E->next()) {
		E->get()->_propagate_transform_changed();
	}

	dirty |= DIRTY_GLOBAL;
}

void IKTransform3D::_update_local_transform() const {
	local_transform.basis = rotation.scaled(scale);

	dirty &= ~DIRTY_LOCAL;
}

void IKTransform3D::set_translation(const Vector3 &p_translation) {
	local_transform.origin = p_translation;
	_propagate_transform_changed();
}

void IKTransform3D::set_rotation(const Basis &p_rot) {
	if (dirty & DIRTY_VECTORS) {
		scale = local_transform.basis.get_scale();
		dirty &= ~DIRTY_VECTORS;
	}

	rotation = p_rot;
	dirty |= DIRTY_LOCAL;
	_propagate_transform_changed();
}

void IKTransform3D::set_scale(const Vector3 &p_scale) {
	if (dirty & DIRTY_VECTORS) {
		rotation = local_transform.basis.get_rotation();
		dirty &= ~DIRTY_VECTORS;
	}

	scale = p_scale;
	dirty |= DIRTY_LOCAL;
	_propagate_transform_changed();
}

Vector3 IKTransform3D::get_translation() const {
	return local_transform.origin;
}

Basis IKTransform3D::get_rotation() const {
	if (dirty & DIRTY_VECTORS) {
		scale = local_transform.basis.get_scale();
		rotation = local_transform.basis.get_rotation();

		dirty &= ~DIRTY_VECTORS;
	}

	return rotation;
}

Vector3 IKTransform3D::get_scale() const {
	if (dirty & DIRTY_VECTORS) {
		scale = local_transform.basis.get_scale();
		rotation = local_transform.basis.get_rotation();

		dirty &= ~DIRTY_VECTORS;
	}

	return scale;
}

void IKTransform3D::set_transform(const Transform3D &p_transform) {
	if (local_transform == p_transform)
		return;
	local_transform = p_transform;
	dirty |= DIRTY_VECTORS;
	_propagate_transform_changed();
}

void IKTransform3D::set_global_transform(const Transform3D &p_transform) {
	Transform3D xform = parent ?
								parent->get_global_transform().affine_inverse() * p_transform :
								p_transform;
	set_transform(xform);
}

Transform3D IKTransform3D::get_transform() const {
	if (dirty & DIRTY_LOCAL) {
		_update_local_transform();
	}

	return local_transform;
}

Transform3D IKTransform3D::get_global_transform() const {
	if (dirty & DIRTY_GLOBAL) {
		if (dirty & DIRTY_LOCAL) {
			_update_local_transform();
		}

		if (parent) {
			global_transform = parent->get_global_transform() * local_transform;
		} else {
			global_transform = local_transform;
		}

		if (disable_scale) {
			global_transform.basis.orthonormalize();
		}

		dirty &= ~DIRTY_GLOBAL;
	}

	return global_transform;
}

void IKTransform3D::set_disable_scale(bool p_enabled) {
	disable_scale = p_enabled;
}

bool IKTransform3D::is_scale_disabled() const {
	return disable_scale;
}

void IKTransform3D::set_parent(IKTransform3D *p_parent) {
	parent = p_parent;
	parent->children.push_back(this);
	_propagate_transform_changed();
}

IKTransform3D *IKTransform3D::get_parent() const {
	return parent;
}

void IKTransform3D::rotate_object_local(const Vector3 &p_axis, real_t p_angle) {
	Transform3D t = get_transform();
	t.basis.rotate_local(p_axis, p_angle);
	set_transform(t);
}

void IKTransform3D::rotate(const Vector3 &p_axis, real_t p_angle) {
	Transform3D t = get_transform();
	t.basis.rotate(p_axis, p_angle);
	set_transform(t);
}

void IKTransform3D::rotate_x(real_t p_angle) {
	Transform3D t = get_transform();
	t.basis.rotate(Vector3(1, 0, 0), p_angle);
	set_transform(t);
}

void IKTransform3D::rotate_y(real_t p_angle) {
	Transform3D t = get_transform();
	t.basis.rotate(Vector3(0, 1, 0), p_angle);
	set_transform(t);
}

void IKTransform3D::rotate_z(real_t p_angle) {
	Transform3D t = get_transform();
	t.basis.rotate(Vector3(0, 0, 1), p_angle);
	set_transform(t);
}

void IKTransform3D::translate(const Vector3 &p_offset) {
	Transform3D t = get_transform();
	t.translate(p_offset);
	set_transform(t);
}

void IKTransform3D::translate_object_local(const Vector3 &p_offset) {
	Transform3D t = get_transform();

	Transform3D s;
	s.translate(p_offset);
	set_transform(t * s);
}

void IKTransform3D::scale_object_local(const Vector3 &p_scale) {
	Transform3D t = get_transform();
	t.basis.scale_local(p_scale);
	set_transform(t);
}

void IKTransform3D::global_rotate(const Vector3 &p_axis, real_t p_angle) {
	Transform3D t = get_global_transform();
	t.basis.rotate(p_axis, p_angle);
	set_global_transform(t);
}

void IKTransform3D::global_scale(const Vector3 &p_scale) {
	Transform3D t = get_global_transform();
	t.basis.scale(p_scale);
	set_global_transform(t);
}

void IKTransform3D::global_translate(const Vector3 &p_offset) {
	Transform3D t = get_global_transform();
	t.origin += p_offset;
	set_global_transform(t);
}

Vector3 IKTransform3D::to_local(const Vector3 &p_global) const {
	return get_global_transform().affine_inverse().xform(p_global);
}

Vector3 IKTransform3D::to_global(const Vector3 &p_local) const {
	return get_global_transform().xform(p_local);
}

void IKTransform3D::orthonormalize() {
	Transform3D t = get_transform();
	t.orthonormalize();
	set_transform(t);
}

void IKTransform3D::set_identity() {
	set_transform(Transform3D());
}
