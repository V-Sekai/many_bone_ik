/**************************************************************************/
/*  test_qcp_validation.h                                                 */
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
#include "tests/test_macros.h"

namespace TestQCPValidation {

// Custom assertion macros for QCP-specific checks
#define CHECK_ROTATION_EQUIVALENT(actual, expected, epsilon) \
	CHECK(Math::abs(Math::abs(actual.dot(expected)) - 1.0) < epsilon)

#define CHECK_ROTATION_NORMALIZED(rotation) \
	CHECK(rotation.is_normalized())

#define CHECK_POINT_ALIGNMENT(rotation, translation, moved, target) \
	CHECK(QuaternionCharacteristicPolynomial::validate_point_alignment(rotation, translation, moved, target, 1e-3))

#define CHECK_DISTANCE_PRESERVATION(rotation, moved) \
	CHECK(QuaternionCharacteristicPolynomial::validate_distance_preservation(rotation, moved))

#define CHECK_ORTHOGONALITY(rotation) \
	CHECK(QuaternionCharacteristicPolynomial::validate_orthogonality(rotation))

#define CHECK_IDENTITY_ROTATION(rotation, epsilon) \
	CHECK_ROTATION_EQUIVALENT(rotation, Quaternion(), epsilon)

#define CHECK_TRANSLATION_ZERO(translation) \
	CHECK(translation.is_zero_approx())

#define CHECK_FINITE_VALUES(rotation, translation) \
	do {                                           \
		CHECK(rotation.is_finite());               \
		CHECK(translation.is_finite());            \
	} while (0)

// Validation helper functions
inline void validate_qcp_result(const Quaternion &rotation, const Vector3 &translation,
		const Quaternion &expected_rotation,
		const Vector3 &expected_translation,
		double epsilon = 1e-6) {
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, epsilon);
	CHECK((translation - expected_translation).length() < epsilon);
}

inline void validate_identity_result(const Quaternion &rotation, const Vector3 &translation, double epsilon = 1e-6) {
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_IDENTITY_ROTATION(rotation, epsilon);
	CHECK_TRANSLATION_ZERO(translation);
}

inline void validate_geometric_properties(const Quaternion &rotation,
		const PackedVector3Array &moved_points,
		const PackedVector3Array &target_points,
		const Vector3 &translation) {
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_POINT_ALIGNMENT(rotation, translation, moved_points, target_points);
	CHECK_DISTANCE_PRESERVATION(rotation, moved_points);
	CHECK_ORTHOGONALITY(rotation);
}

inline void validate_transformation_accuracy(const Quaternion &rotation,
		const Vector3 &translation,
		const PackedVector3Array &moved_points,
		const PackedVector3Array &target_points,
		double epsilon = 1e-3) {
	// Verify the transformation actually works by applying it
	bool points_align = true;
	for (int i = 0; i < moved_points.size(); ++i) {
		Vector3 transformed_point = rotation.xform(moved_points[i]) + translation;
		if ((transformed_point - target_points[i]).length() > epsilon) {
			points_align = false;
			break;
		}
	}
	CHECK(points_align);
}

inline void validate_canonical_form_consistency(const PackedVector3Array &moved_points,
		const PackedVector3Array &target_points,
		const Vector<double> &weights,
		bool translate = false,
		double epsilon = 1e-6) {
	// Run multiple times to check consistency
	Array result1 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Array result2 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Array result3 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);

	Quaternion rotation1 = result1[0];
	Quaternion rotation2 = result2[0];
	Quaternion rotation3 = result3[0];

	// All quaternions should be canonicalized (w >= 0) and consistent
	CHECK(rotation1.w >= -epsilon);
	CHECK(rotation2.w >= -epsilon);
	CHECK(rotation3.w >= -epsilon);

	// Results should be identical (or equivalent)
	CHECK_ROTATION_EQUIVALENT(rotation1, rotation2, epsilon);
	CHECK_ROTATION_EQUIVALENT(rotation1, rotation3, epsilon);
}

inline void validate_rmsd_properties(const Quaternion &rotation,
		const Vector3 &translation,
		const PackedVector3Array &moved_points,
		const PackedVector3Array &target_points) {
	double rmsd = QuaternionCharacteristicPolynomial::calculate_rmsd(rotation, translation, moved_points, target_points);
	CHECK(rmsd >= 0.0); // RMSD should be non-negative
	CHECK(rmsd < 1e10); // Should be reasonable (not infinite)
}

inline void validate_single_point_behavior(const Vector3 &moved_point,
		const Vector3 &target_point,
		bool translate,
		double epsilon = 1e-6) {
	PackedVector3Array moved_points;
	moved_points.push_back(moved_point);

	PackedVector3Array target_points;
	target_points.push_back(target_point);

	Vector<double> weights;
	weights.push_back(1.0);

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation_result = result[1];

	if (translate) {
		// Should use identity rotation and correct translation
		CHECK_IDENTITY_ROTATION(rotation, epsilon);
		Vector3 expected_translation = target_point - moved_point;
		CHECK((translation_result - expected_translation).length() < epsilon);
	} else {
		// Should calculate the rotation needed to align the points
		CHECK_ROTATION_NORMALIZED(rotation);
		CHECK_TRANSLATION_ZERO(translation_result);

		// Direct geometric test: does the rotation actually transform the point correctly?
		Vector3 transformed_point = rotation.xform(moved_point);

		// For unit vectors, check if they align after transformation
		if (moved_point.length() > 1e-10 && target_point.length() > 1e-10) {
			Vector3 transformed_normalized = transformed_point.normalized();
			Vector3 target_normalized = target_point.normalized();

			// Use more reasonable geometric tolerance for single point rotations
			double geometric_tolerance = 0.1; // 10cm tolerance for normalized vectors
			CHECK((transformed_normalized - target_normalized).length() < geometric_tolerance);
		}
	}
}

} // namespace TestQCPValidation
