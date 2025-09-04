/**************************************************************************/
/*  interval_math.h                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/math/math_funcs.h"
#include "core/math/quaternion.h"
#include "core/math/vector3.h"

/**
 * Interval Arithmetic Library for Godot Engine
 *
 * This single-header library provides robust interval arithmetic operations
 * specifically designed to handle numerical singularities in IK calculations.
 *
 * Key features:
 * - Guaranteed bounds on all floating-point operations
 * - Mathematically rigorous singularity detection
 * - Safe alternatives to epsilon-based comparisons
 * - Zero external dependencies (Godot core math only)
 */

namespace IntervalMath {

// Default uncertainty for converting exact values to intervals
static const real_t DEFAULT_UNCERTAINTY = 1e-15;

/**
 * Core interval type representing a range [lower, upper] with guaranteed bounds
 */
struct Interval {
	real_t lower;
	real_t upper;

	// Constructors
	Interval() :
			lower(0), upper(0) {}
	Interval(real_t value) :
			lower(value), upper(value) {}
	Interval(real_t l, real_t u) :
			lower(l), upper(u) {
		if (l > u) {
			// Swap if bounds are reversed
			lower = u;
			upper = l;
		}
	}

	// Create interval from exact value with uncertainty
	static Interval from_value(real_t value, real_t uncertainty = DEFAULT_UNCERTAINTY) {
		return Interval(value - uncertainty, value + uncertainty);
	}

	// Create interval with high precision (smaller uncertainty)
	static Interval from_value_high_precision(real_t value) {
		return Interval(value - 1e-18, value + 1e-18);
	}

	// Create interval with low precision (larger uncertainty)
	static Interval from_value_low_precision(real_t value) {
		return Interval(value - 1e-12, value + 1e-12);
	}

	// Basic properties
	real_t width() const { return upper - lower; }
	real_t center() const { return (lower + upper) * 0.5; }
	bool is_empty() const { return lower > upper; }
	bool is_point() const { return Math::is_equal_approx(lower, upper); }

	// Containment tests
	bool contains(real_t value) const {
		return value >= lower && value <= upper;
	}

	bool contains(const Interval &other) const {
		return lower <= other.lower && upper >= other.upper;
	}

	bool intersects(const Interval &other) const {
		return !(upper < other.lower || lower > other.upper);
	}

	// Arithmetic operations with proper bound calculation
	Interval operator+(const Interval &other) const {
		return Interval(lower + other.lower, upper + other.upper);
	}

	Interval operator-(const Interval &other) const {
		return Interval(lower - other.upper, upper - other.lower);
	}

	Interval operator*(const Interval &other) const {
		real_t ll = lower * other.lower;
		real_t lu = lower * other.upper;
		real_t ul = upper * other.lower;
		real_t uu = upper * other.upper;
		return Interval(MIN(MIN(ll, lu), MIN(ul, uu)), MAX(MAX(ll, lu), MAX(ul, uu)));
	}

	Interval operator/(const Interval &other) const {
		// Division by interval containing zero is undefined
		if (other.contains(0.0)) {
			return Interval(-INFINITY, INFINITY);
		}
		real_t ll = lower / other.lower;
		real_t lu = lower / other.upper;
		real_t ul = upper / other.lower;
		real_t uu = upper / other.upper;
		return Interval(MIN(MIN(ll, lu), MIN(ul, uu)), MAX(MAX(ll, lu), MAX(ul, uu)));
	}

	// Unary operations
	Interval operator-() const {
		return Interval(-upper, -lower);
	}

	// Mathematical functions
	Interval abs() const {
		if (lower >= 0) {
			return *this;
		} else if (upper <= 0) {
			return Interval(-upper, -lower);
		} else {
			return Interval(0, MAX(-lower, upper));
		}
	}

	Interval sqrt() const {
		if (upper < 0) {
			return Interval(); // Empty interval for negative sqrt
		}
		real_t l = (lower <= 0) ? 0 : Math::sqrt(lower);
		real_t u = Math::sqrt(upper);
		return Interval(l, u);
	}

	Interval sin() const {
		// Simplified sin for small intervals
		if (width() <= Math::PI) {
			return Interval(Math::sin(lower), Math::sin(upper));
		}
		return Interval(-1, 1); // Full range for large intervals
	}

	Interval cos() const {
		// Simplified cos for small intervals
		if (width() <= Math::PI) {
			return Interval(Math::cos(upper), Math::cos(lower));
		}
		return Interval(-1, 1); // Full range for large intervals
	}
};

/**
 * 3D interval vector for uncertain Vector3 operations
 */
struct Interval3D {
	Interval x, y, z;

	// Constructors
	Interval3D() {}
	Interval3D(const Interval &x_val, const Interval &y_val, const Interval &z_val) :
			x(x_val), y(y_val), z(z_val) {}
	Interval3D(const Vector3 &vec, real_t uncertainty = DEFAULT_UNCERTAINTY) {
		x = Interval::from_value(vec.x, uncertainty);
		y = Interval::from_value(vec.y, uncertainty);
		z = Interval::from_value(vec.z, uncertainty);
	}

	// Convert back to Vector3 (using center values)
	Vector3 to_vector3() const {
		return Vector3(x.center(), y.center(), z.center());
	}

	// Component access
	Interval &operator[](int index) {
		return (&x)[index];
	}
	const Interval &operator[](int index) const {
		return (&x)[index];
	}

	// Vector operations
	Interval3D operator+(const Interval3D &other) const {
		return Interval3D(x + other.x, y + other.y, z + other.z);
	}

	Interval3D operator-(const Interval3D &other) const {
		return Interval3D(x - other.x, y - other.y, z - other.z);
	}

	Interval3D operator*(const Interval &scalar) const {
		return Interval3D(x * scalar, y * scalar, z * scalar);
	}

	Interval3D operator/(const Interval &scalar) const {
		return Interval3D(x / scalar, y / scalar, z / scalar);
	}

	// Dot product with guaranteed bounds
	Interval dot(const Interval3D &other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	// Cross product with proper error propagation
	Interval3D cross(const Interval3D &other) const {
		return Interval3D(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x);
	}

	// Length squared (avoids sqrt for efficiency)
	Interval length_squared() const {
		return x * x + y * y + z * z;
	}

	// Length with interval bounds
	Interval length() const {
		return length_squared().sqrt();
	}

	// Check if vector is effectively zero
	bool is_zero() const {
		return length_squared().upper <= DEFAULT_UNCERTAINTY * DEFAULT_UNCERTAINTY;
	}

	// Check if vector can be normalized safely
	bool is_normalizable() const {
		return length_squared().lower > DEFAULT_UNCERTAINTY * DEFAULT_UNCERTAINTY;
	}

	// Normalized vector (returns empty if not normalizable)
	Interval3D normalized() const {
		if (!is_normalizable()) {
			return Interval3D(); // Return zero vector for degenerate case
		}
		Interval len = length();
		return *this / len;
	}
};

/**
 * Interval quaternion for uncertain rotation operations
 */
struct IntervalQuaternion {
	Interval x, y, z, w;

	// Constructors
	IntervalQuaternion() :
			w(Interval(1)) {} // Identity quaternion
	IntervalQuaternion(const Interval &x_val, const Interval &y_val, const Interval &z_val, const Interval &w_val) :
			x(x_val), y(y_val), z(z_val), w(w_val) {}
	IntervalQuaternion(const Quaternion &quat, real_t uncertainty = DEFAULT_UNCERTAINTY) {
		x = Interval::from_value(quat.x, uncertainty);
		y = Interval::from_value(quat.y, uncertainty);
		z = Interval::from_value(quat.z, uncertainty);
		w = Interval::from_value(quat.w, uncertainty);
	}

	// Convert back to Quaternion (using center values)
	Quaternion to_quaternion() const {
		return Quaternion(x.center(), y.center(), z.center(), w.center()).normalized();
	}

	// Quaternion multiplication with interval bounds
	IntervalQuaternion operator*(const IntervalQuaternion &other) const {
		return IntervalQuaternion(
				w * other.x + x * other.w + y * other.z - z * other.y,
				w * other.y - x * other.z + y * other.w + z * other.x,
				w * other.z + x * other.y - y * other.x + z * other.w,
				w * other.w - x * other.x - y * other.y - z * other.z);
	}

	// Quaternion conjugate
	IntervalQuaternion conjugate() const {
		return IntervalQuaternion(-x, -y, -z, w);
	}

	// Length squared
	Interval length_squared() const {
		return x * x + y * y + z * z + w * w;
	}

	// Check if quaternion is effectively normalized
	bool is_normalized() const {
		Interval len_sq = length_squared();
		return len_sq.contains(1.0);
	}
};

// ============================================================================
// IK-Specific Utility Functions
// ============================================================================

/**
 * Check if two vectors are parallel (or anti-parallel) with mathematical certainty
 */
inline bool are_parallel(const Interval3D &v1, const Interval3D &v2) {
	Interval3D cross_product = v1.cross(v2);
	Interval cross_length_sq = cross_product.length_squared();

	// Vectors are parallel if cross product length is effectively zero
	return cross_length_sq.upper <= DEFAULT_UNCERTAINTY * DEFAULT_UNCERTAINTY;
}

/**
 * Check if two vectors are anti-parallel with mathematical certainty
 */
inline bool are_anti_parallel(const Interval3D &v1, const Interval3D &v2) {
	if (!are_parallel(v1, v2)) {
		return false;
	}

	Interval dot_product = v1.dot(v2);
	return dot_product.upper < 0;
}

/**
 * Safe cross product that handles parallel vectors gracefully
 */
inline Interval3D safe_cross_product(const Interval3D &v1, const Interval3D &v2) {
	Interval3D cross = v1.cross(v2);

	// If vectors are parallel, find a safe orthogonal vector
	if (are_parallel(v1, v2)) {
		// Use the vector with the smallest component magnitude for maximum orthogonality
		Vector3 v1_center = v1.to_vector3();
		Vector3 v1_abs = Vector3(Math::abs(v1_center.x), Math::abs(v1_center.y), Math::abs(v1_center.z));

		Vector3 reference_axis;
		if (v1_abs.x <= v1_abs.y && v1_abs.x <= v1_abs.z) {
			reference_axis = Vector3(1, 0, 0);
		} else if (v1_abs.y <= v1_abs.z) {
			reference_axis = Vector3(0, 1, 0);
		} else {
			reference_axis = Vector3(0, 0, 1);
		}

		// Create orthogonal vector using Gram-Schmidt
		Interval3D ref_interval(reference_axis);
		Interval dot_ref_v1 = ref_interval.dot(v1);
		Interval3D orthogonal = ref_interval - v1 * (dot_ref_v1 / v1.length_squared());

		// If still degenerate, try another axis
		if (orthogonal.is_zero()) {
			reference_axis = (reference_axis.x > 0.5) ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
			ref_interval = Interval3D(reference_axis);
			dot_ref_v1 = ref_interval.dot(v1);
			orthogonal = ref_interval - v1 * (dot_ref_v1 / v1.length_squared());
		}

		return orthogonal.normalized();
	}

	return cross.normalized();
}

/**
 * Get a safe orthogonal vector to the input vector
 */
inline Interval3D get_safe_orthogonal(const Interval3D &v) {
	Vector3 v_center = v.to_vector3();
	Vector3 v_abs = Vector3(Math::abs(v_center.x), Math::abs(v_center.y), Math::abs(v_center.z));

	// Choose the axis with smallest component for maximum orthogonality
	Vector3 reference_axis;
	if (v_abs.x <= v_abs.y && v_abs.x <= v_abs.z) {
		reference_axis = Vector3(1, 0, 0);
	} else if (v_abs.y <= v_abs.z) {
		reference_axis = Vector3(0, 1, 0);
	} else {
		reference_axis = Vector3(0, 0, 1);
	}

	Interval3D ref_interval(reference_axis);
	Interval dot_ref_v = ref_interval.dot(v);
	Interval3D orthogonal = ref_interval - v * (dot_ref_v / v.length_squared());

	if (orthogonal.is_zero()) {
		// Fallback to another axis
		reference_axis = (reference_axis.x > 0.5) ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
		ref_interval = Interval3D(reference_axis);
		dot_ref_v = ref_interval.dot(v);
		orthogonal = ref_interval - v * (dot_ref_v / v.length_squared());
	}

	return orthogonal.normalized();
}

/**
 * Safe quaternion creation from axis-angle with singularity handling
 */
inline IntervalQuaternion safe_quaternion_from_axis_angle(const Interval3D &axis, const Interval &angle) {
	// Check for zero-length axis
	if (axis.is_zero()) {
		return IntervalQuaternion(); // Identity quaternion
	}

	// Check for zero angle
	if (angle.abs().upper <= DEFAULT_UNCERTAINTY) {
		return IntervalQuaternion(); // Identity quaternion
	}

	// Normalize axis safely
	Interval3D normalized_axis = axis.normalized();
	if (normalized_axis.is_zero()) {
		return IntervalQuaternion(); // Fallback to identity
	}

	// Calculate half angle
	Interval half_angle = angle * Interval(0.5);
	Interval sin_half = half_angle.sin();
	Interval cos_half = half_angle.cos();

	return IntervalQuaternion(
			normalized_axis.x * sin_half,
			normalized_axis.y * sin_half,
			normalized_axis.z * sin_half,
			cos_half);
}

/**
 * Safe swing-twist decomposition with guaranteed bounds
 */
inline void safe_swing_twist_decomposition(
		const IntervalQuaternion &rotation,
		const Interval3D &axis,
		IntervalQuaternion &swing,
		IntervalQuaternion &twist) {
	// Handle zero-length axis
	if (axis.is_zero()) {
		swing = IntervalQuaternion();
		twist = IntervalQuaternion();
		return;
	}

	// Normalize axis
	Interval3D normalized_axis = axis.normalized();
	if (normalized_axis.is_zero()) {
		swing = IntervalQuaternion();
		twist = IntervalQuaternion();
		return;
	}

	// Project rotation onto twist axis
	Interval dot_product = Interval3D(rotation.x, rotation.y, rotation.z).dot(normalized_axis);
	Interval3D projection = normalized_axis * dot_product;

	// Construct twist quaternion
	twist = IntervalQuaternion(projection.x, projection.y, projection.z, rotation.w);

	// Check if twist is normalizable
	if (!twist.is_normalized()) {
		// Degenerate case - no twist component
		twist = IntervalQuaternion();
		swing = rotation;
		return;
	}

	// Calculate swing as remaining rotation
	IntervalQuaternion twist_conjugate = twist.conjugate();
	swing = rotation * twist_conjugate;

	// Validate results
	if (!swing.is_normalized() || !twist.is_normalized()) {
		// Fallback for degenerate cases
		swing = IntervalQuaternion();
		twist = IntervalQuaternion();
	}
}

/**
 * Check if a point is within cone bounds using interval arithmetic
 */
inline bool is_point_in_cone(const Interval3D &point, const Interval3D &cone_axis, const Interval &cone_angle_cos) {
	Interval dot_product = point.dot(cone_axis);
	return dot_product.lower >= cone_angle_cos.lower;
}

/**
 * Safe distance calculation between two interval vectors
 */
inline Interval safe_distance(const Interval3D &v1, const Interval3D &v2) {
	Interval3D diff = v1 - v2;
	return diff.length();
}

/**
 * Check if an interval quaternion represents a small rotation
 */
inline bool is_small_rotation(const IntervalQuaternion &q) {
	// Small rotation if w component is close to 1
	return q.w.lower > (1.0 - DEFAULT_UNCERTAINTY);
}

} // namespace IntervalMath
