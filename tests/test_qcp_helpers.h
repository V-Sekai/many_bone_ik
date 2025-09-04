/**************************************************************************/
/*  test_qcp_helpers.h                                                    */
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

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/math/quaternion.h"
#include "modules/many_bone_ik/src/math/qcp.h"

namespace TestQCPHelpers {

// Common point set generators
inline PackedVector3Array create_basic_point_set() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 0, 0));
	points.push_back(Vector3(0, 1, 0));
	points.push_back(Vector3(0, 0, 1));
	return points;
}

inline PackedVector3Array create_extended_point_set() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 0, 0));
	points.push_back(Vector3(0, 1, 0));
	points.push_back(Vector3(0, 0, 1));
	points.push_back(Vector3(1, 1, 1));
	return points;
}

inline PackedVector3Array create_simple_point_set() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 2, 3));
	points.push_back(Vector3(4, 5, 6));
	points.push_back(Vector3(7, 8, 9));
	return points;
}

inline PackedVector3Array create_collinear_points() {
	PackedVector3Array points;
	points.push_back(Vector3(0, 0, 0));
	points.push_back(Vector3(1, 0, 0));
	points.push_back(Vector3(2, 0, 0));
	points.push_back(Vector3(3, 0, 0));
	return points;
}

inline PackedVector3Array create_identical_points() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 1, 1));
	points.push_back(Vector3(1, 1, 1));
	points.push_back(Vector3(1, 1, 1));
	return points;
}

inline PackedVector3Array create_opposite_vector_pairs() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 0, 0));
	points.push_back(Vector3(-1, 0, 0));
	points.push_back(Vector3(0, 1, 0));
	points.push_back(Vector3(0, -1, 0));
	return points;
}

// Weight array generators
inline Vector<double> create_uniform_weights(int count, double weight = 1.0) {
	Vector<double> weights;
	for (int i = 0; i < count; ++i) {
		weights.push_back(weight);
	}
	return weights;
}

inline Vector<double> create_zero_weights(int count) {
	return create_uniform_weights(count, 0.0);
}

inline Vector<double> create_weighted_with_outlier(int count, double normal_weight = 1.0, double outlier_weight = 0.1) {
	Vector<double> weights = create_uniform_weights(count - 1, normal_weight);
	weights.push_back(outlier_weight);
	return weights;
}

// Transformation application helpers
inline PackedVector3Array apply_transformation(const PackedVector3Array &points,
		const Quaternion &rotation,
		const Vector3 &translation = Vector3()) {
	PackedVector3Array transformed_points;
	for (int i = 0; i < points.size(); ++i) {
		transformed_points.push_back(rotation.xform(points[i]) + translation);
	}
	return transformed_points;
}

inline PackedVector3Array apply_translation_only(const PackedVector3Array &points, const Vector3 &translation) {
	PackedVector3Array translated_points;
	for (int i = 0; i < points.size(); ++i) {
		translated_points.push_back(points[i] + translation);
	}
	return translated_points;
}

// QCP computation wrapper
inline Array compute_qcp_transformation(const PackedVector3Array &moved_points,
		const PackedVector3Array &target_points,
		bool translate = false,
		double epsilon = 1e-6) {
	Vector<double> weights = create_uniform_weights(moved_points.size());
	return QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
}

inline Array compute_qcp_transformation_weighted(const PackedVector3Array &moved_points,
		const PackedVector3Array &target_points,
		const Vector<double> &weights,
		bool translate = false,
		double epsilon = 1e-6) {
	return QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
}

} // namespace TestQCPHelpers
