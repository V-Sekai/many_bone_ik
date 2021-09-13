/*************************************************************************/
/*  ik_transform.h                                                       */
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

#ifndef IK_TRANSFORM_H
#define IK_TRANSFORM_H

#include "core/math/transform_3d.h"
#include "core/templates/list.h"

class IKTransform3D {
	enum TransformDirty {
		DIRTY_NONE = 0,
		DIRTY_VECTORS = 1,
		DIRTY_LOCAL = 2,
		DIRTY_GLOBAL = 4
	};

	mutable Transform3D global_transform;
	mutable Transform3D local_transform;
	mutable Basis rotation;
	mutable Vector3 scale = Vector3(1, 1, 1);

	mutable int dirty = DIRTY_NONE;

	IKTransform3D *parent = nullptr;
	List<IKTransform3D *> children;

	bool disable_scale = false;

	void _propagate_transform_changed();
	void _update_local_transform() const;

public:
	void set_translation(const Vector3 &p_translation);
	void set_rotation(const Basis &p_euler_rad);
	void set_scale(const Vector3 &p_scale);
	Vector3 get_translation() const;
	Basis get_rotation() const;
	Vector3 get_scale() const;

	void set_transform(const Transform3D &p_transform);
	void set_global_transform(const Transform3D &p_transform);
	Transform3D get_transform() const;
	Transform3D get_global_transform() const;

	void set_disable_scale(bool p_enabled);
	bool is_scale_disabled() const;

	void set_parent(IKTransform3D *p_parent);
	IKTransform3D *get_parent() const;

	void rotate(const Vector3 &p_axis, real_t p_angle);
	void rotate_x(real_t p_angle);
	void rotate_y(real_t p_angle);
	void rotate_z(real_t p_angle);
	void translate(const Vector3 &p_offset);

	void rotate_object_local(const Vector3 &p_axis, real_t p_angle);
	void scale_object_local(const Vector3 &p_scale);
	void translate_object_local(const Vector3 &p_offset);

	void global_rotate(const Vector3 &p_axis, real_t p_angle);
	void global_scale(const Vector3 &p_scale);
	void global_translate(const Vector3 &p_offset);

	Vector3 to_local(const Vector3 &p_global) const;
	Vector3 to_global(const Vector3 &p_local) const;

	void orthonormalize();
	void set_identity();
};

#endif // IK_TRANSFORM_H