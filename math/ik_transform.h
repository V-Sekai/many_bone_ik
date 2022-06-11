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

#include "core/templates/list.h"

#include "core/math/transform_3d.h"
#include "core/object/ref_counted.h"

class IKTransform3D : public RefCounted {
	GDCLASS(IKTransform3D, RefCounted);
	friend class IKBone3D;

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

	Ref<IKTransform3D> parent;
	List<Ref<IKTransform3D>> children;

	bool disable_scale = false;

	void _propagate_transform_changed();
	void _update_local_transform() const;

	float chirality = 1.0;

public:
	void set_transform(const Transform3D &p_transform);
	void set_global_transform(const Transform3D &p_transform);
	Transform3D get_transform() const;
	Transform3D get_global_transform() const;

	void set_disable_scale(bool p_enabled);
	bool is_scale_disabled() const;

	void set_parent(Ref<IKTransform3D> p_parent);
	Ref<IKTransform3D> get_parent() const;

	Vector3 to_local(const Vector3 &p_global) const;
	Vector3 to_global(const Vector3 &p_local) const;
	void rotate_local_with_global(Quaternion p_q);
	real_t get_global_chirality() {
		return chirality;
	}
	real_t set_global_chirality(real_t p_chirality) {
		bool is_value = Math::is_equal_approx(p_chirality, real_t(-1.0)) || Math::is_equal_approx(p_chirality, real_t(1.0));
		ERR_FAIL_COND_V(!is_value, real_t(1.0));
		chirality = p_chirality;
		return p_chirality < 0 ? false : true;
	}
	void orthonormalize();
	void set_identity();
};

#endif // IK_TRANSFORM_H
