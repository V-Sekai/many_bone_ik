/*************************************************************************/
/*  transform_interpolator.h                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef TRANSFORM_INTERPOLATE_H
#define TRANSFORM_INTERPOLATE_H

#include "core/math/math_defs.h"
#include "core/math/quaternion.h"
#include "core/math/transform_3d.h"
#include "core/math/vector3.h"


class TransformInterpolate3D {
	Transform3D transform;
	// Keep all the functions for fixed timestep interpolation together.
	// There are two stages involved:
	// Finding a method, for determining the interpolation method between two
	// keyframes (which are physics ticks).
	// And applying that pre-determined method.

	// Pre-determining the method makes sense because it is expensive and often
	// several frames may occur between each physics tick, which will make it cheaper
	// than performing every frame.
public:
	enum Method {
		INTERP_LERP,
		INTERP_SLERP,
		INTERP_SCALED_SLERP,
	};

private:
	real_t _vector3_normalize(Vector3 &p_vec) const;
	Vector3 _basis_orthonormalize(Basis &r_basis) const;
	real_t vector3_sum(const Vector3 &p_pt) const { return p_pt.x + p_pt.y + p_pt.z; }
	Method _test_basis(Basis p_basis, bool r_needed_normalize, Quaternion &r_quat) const;
	Basis _basis_slerp_unchecked(Basis p_from, Basis p_to, real_t p_fraction) const;
	Quaternion _quat_slerp_unchecked(const Quaternion &p_from, const Quaternion &p_to, real_t p_fraction) const;
	Quaternion _basis_to_quaternion_unchecked(const Basis &p_basis) const;
	bool _basis_is_orthogonal(const Basis &p_basis, real_t p_epsilon = 0.01) const;
	bool _basis_is_orthogonal_any_scale(const Basis &p_basis) const;

	void interpolate_transform_3d_linear(const Transform3D &p_prev, const Transform3D &p_curr, Transform3D &r_result, real_t p_fraction) const;
	void interpolate_transform_3d_scaled_slerp(const Transform3D &p_prev, const Transform3D &p_curr, Transform3D &r_result, real_t p_fraction) const;

private:
	void interpolate_transform_3d_via_method(const Transform3D &p_prev, const Transform3D &p_curr, Transform3D &r_result, real_t p_fraction, Method p_method) const;

	real_t checksum_transform_3d(const Transform3D &p_transform) const;
	Method find_method(const Basis &p_a, const Basis &p_b) const;
public:
	void set(const Transform3D &p_transform) {
		transform = p_transform;
	}
	Transform3D get() const {
		return transform;
	}
	Transform3D interpolate_with(const Transform3D &p_curr, real_t p_fraction = 1.0f) const;
};

#endif // TRANSFORM_INTERPOLATOR_H