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

void IKTransform::_propagate_transform_changed() {
    for (List<IKTransform *>::Element *E = children.front(); E; E = E->next()) {
		E->get()->_propagate_transform_changed();
	}

    dirty |= DIRTY_GLOBAL;
}

void IKTransform::_update_local_transform() const {
	local_transform.basis.set_euler_scale(rotation, scale);

	dirty &= ~DIRTY_LOCAL;
}

void IKTransform::set_translation(const Vector3 &p_translation) {
    local_transform.origin = p_translation;
	_propagate_transform_changed();
}

void IKTransform::set_rotation(const Vector3 &p_euler_rad) {
	if (dirty & DIRTY_VECTORS) {
		scale = local_transform.basis.get_scale();
		dirty &= ~DIRTY_VECTORS;
	}

	rotation = p_euler_rad;
	dirty |= DIRTY_LOCAL;
	_propagate_transform_changed();
}

void IKTransform::set_rotation_degrees(const Vector3 &p_euler_deg) {
	set_rotation(p_euler_deg * (Math_PI / 180.0));
}

void IKTransform::set_scale(const Vector3 &p_scale) {
	if (dirty & DIRTY_VECTORS) {
		rotation = local_transform.basis.get_rotation();
		dirty &= ~DIRTY_VECTORS;
	}

	scale = p_scale;
	dirty |= DIRTY_LOCAL;
	_propagate_transform_changed();
}

Vector3 IKTransform::get_translation() const {
	return local_transform.origin;
}

Vector3 IKTransform::get_rotation() const {
	if (dirty & DIRTY_VECTORS) {
		scale = local_transform.basis.get_scale();
		rotation = local_transform.basis.get_rotation();

		dirty &= ~DIRTY_VECTORS;
	}

	return rotation;
}

Vector3 IKTransform::get_rotation_degrees() const {
	return get_rotation() * (180.0 / Math_PI);
}

Vector3 IKTransform::get_scale() const {
	if (dirty & DIRTY_VECTORS) {
		scale = local_transform.basis.get_scale();
		rotation = local_transform.basis.get_rotation();

		dirty &= ~DIRTY_VECTORS;
	}

	return scale;
}

void IKTransform::set_transform(const Transform &p_transform) {
	if (local_transform == p_transform)
		return;
	local_transform = p_transform;
	dirty |= DIRTY_VECTORS;
	_propagate_transform_changed();
}

void IKTransform::set_global_transform(const Transform &p_transform) {
	Transform xform = parent ?
                parent->get_global_transform().affine_inverse() * p_transform :
                p_transform;
	set_transform(xform);
}

Transform IKTransform::get_transform() const {
	if (dirty & DIRTY_LOCAL) {
		_update_local_transform();
	}

	return local_transform;
}

Transform IKTransform::get_global_transform() const {
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

void IKTransform::set_disable_scale(bool p_enabled) {
	disable_scale = p_enabled;
}

bool IKTransform::is_scale_disabled() const {
	return disable_scale;
}

void IKTransform::set_parent(IKTransform *p_parent) {
	parent = p_parent;
	parent->children.push_back(this);
	_propagate_transform_changed();
}

IKTransform *IKTransform::get_parent() const {
	return parent;
}

void IKTransform::rotate_object_local(const Vector3 &p_axis, real_t p_angle) {
	Transform t = get_transform();
	t.basis.rotate_local(p_axis, p_angle);
	set_transform(t);
}

void IKTransform::rotate(const Vector3 &p_axis, real_t p_angle) {
	Transform t = get_transform();
	t.basis.rotate(p_axis, p_angle);
	set_transform(t);
}

void IKTransform::rotate_x(real_t p_angle) {
	Transform t = get_transform();
	t.basis.rotate(Vector3(1, 0, 0), p_angle);
	set_transform(t);
}

void IKTransform::rotate_y(real_t p_angle) {
	Transform t = get_transform();
	t.basis.rotate(Vector3(0, 1, 0), p_angle);
	set_transform(t);
}

void IKTransform::rotate_z(real_t p_angle) {
	Transform t = get_transform();
	t.basis.rotate(Vector3(0, 0, 1), p_angle);
	set_transform(t);
}

void IKTransform::translate(const Vector3 &p_offset) {
	Transform t = get_transform();
	t.translate(p_offset);
	set_transform(t);
}

void IKTransform::translate_object_local(const Vector3 &p_offset) {
	Transform t = get_transform();

	Transform s;
	s.translate(p_offset);
	set_transform(t * s);
}

void IKTransform::scale_object_local(const Vector3 &p_scale) {
	Transform t = get_transform();
	t.basis.scale_local(p_scale);
	set_transform(t);
}

void IKTransform::global_rotate(const Vector3 &p_axis, real_t p_angle) {
	Transform t = get_global_transform();
	t.basis.rotate(p_axis, p_angle);
	set_global_transform(t);
}

void IKTransform::global_scale(const Vector3 &p_scale) {
	Transform t = get_global_transform();
	t.basis.scale(p_scale);
	set_global_transform(t);
}

void IKTransform::global_translate(const Vector3 &p_offset) {
	Transform t = get_global_transform();
	t.origin += p_offset;
	set_global_transform(t);
}

Vector3 IKTransform::to_local(const Vector3 &p_global) const {
	return get_global_transform().affine_inverse().xform(p_global);
}

Vector3 IKTransform::to_global(const Vector3 &p_local) const {
	return get_global_transform().xform(p_local);
}

void IKTransform::orthonormalize() {
	Transform t = get_transform();
	t.orthonormalize();
	set_transform(t);
}

void IKTransform::set_identity() {
	set_transform(Transform());
}
