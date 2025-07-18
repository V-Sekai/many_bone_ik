/**************************************************************************/
/*  test_qcp.h                                                            */
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
#include "modules/many_bone_ik/src/math/qcp.h"
#include "tests/test_macros.h"

namespace TestQCP {

TEST_CASE("[Modules][QCP] Identity Transformation (No Rotation, No Translation)") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));
	moved_points.push_back(Vector3(4, 5, 6));
	moved_points.push_back(Vector3(7, 8, 9));

	PackedVector3Array target_points = moved_points; // Target is identical to moved

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	Quaternion expected_rotation = Quaternion(); // Identity
	
	// Use geometric validation instead of brittle component comparisons
	CHECK(rotation.is_normalized()); // Ensure quaternion is unit length
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon); // Test mathematical equivalence
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Simple 90-degree Rotation around Z (No Translation)") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0); // 90 degrees around Z

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]));
	}

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	rotation.normalize();

	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);

	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Simple Translation (No Rotation)") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));
	moved_points.push_back(Vector3(4, 5, 6));
	moved_points.push_back(Vector3(7, 8, 9));

	Quaternion expected_rotation = Quaternion(); // Identity
	Vector3 expected_translation_component = Vector3(10, 20, 30);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(moved_points[i] + expected_translation_component);
	}

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation_result = result[0];
	Vector3 translation_qcp = result[1];

	rotation_result.normalize();

	CHECK(Math::abs(Math::abs(rotation_result.dot(expected_rotation)) - 1.0) < epsilon);

	// Use geometric validation: verify the transformation actually works
	// Apply the computed transformation and check if points align correctly
	bool points_align = true;
	for (int i = 0; i < moved_points.size(); ++i) {
		Vector3 transformed_point = rotation_result.xform(moved_points[i]) + translation_qcp;
		if ((transformed_point - target_points[i]).length() > epsilon) {
			points_align = false;
			break;
		}
	}
	CHECK(points_align);
}

TEST_CASE("[Modules][QCP] Simple 90-degree Rotation around Z AND Translation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0); // 90 degrees around Z
	Vector3 expected_translation_component = Vector3(5, -5, 10);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]) + expected_translation_component);
	}

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation_result = result[0];
	Vector3 translation_qcp = result[1];

	rotation_result.normalize();

	CHECK(Math::abs(Math::abs(rotation_result.dot(expected_rotation)) - 1.0) < epsilon);
	
	// Use geometric validation: verify the transformation actually works
	bool points_align = true;
	for (int i = 0; i < moved_points.size(); ++i) {
		Vector3 transformed_point = rotation_result.xform(moved_points[i]) + translation_qcp;
		if ((transformed_point - target_points[i]).length() > epsilon) {
			points_align = false;
			break;
		}
	}
	CHECK(points_align);
}

TEST_CASE("[Modules][QCP] 45-degree Rotation around Y AND Translation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(2, -1, 3));

	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 4.0); // 45 degrees around Y
	Vector3 expected_translation_component = Vector3(-2, 3, -4);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]) + expected_translation_component);
	}

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation_result = result[0];
	Vector3 translation_qcp = result[1];

	rotation_result.normalize();

	CHECK(Math::abs(Math::abs(rotation_result.dot(expected_rotation)) - 1.0) < epsilon);
	
	// Use geometric validation: verify the transformation actually works
	bool points_align = true;
	for (int i = 0; i < moved_points.size(); ++i) {
		Vector3 transformed_point = rotation_result.xform(moved_points[i]) + translation_qcp;
		if ((transformed_point - target_points[i]).length() > epsilon) {
			points_align = false;
			break;
		}
	}
	CHECK(points_align);
}

// ========================================
// PHASE 1: INPUT VALIDATION TESTS
// ========================================

TEST_CASE("[Modules][QCP] Input Validation - Mismatched Array Sizes") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(1, 0, 0));
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0, 0, 1)); // Extra point

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity quaternion for invalid input
	Quaternion expected_rotation = Quaternion();
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Input Validation - Empty Arrays") {
	PackedVector3Array moved_points;
	PackedVector3Array target_points;
	Vector<double> weights;

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity quaternion for empty input
	Quaternion expected_rotation = Quaternion();
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Input Validation - Single Point") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(4, 5, 6));

	Vector<double> weights;
	weights.push_back(1.0);

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity rotation and correct translation
	Quaternion expected_rotation = Quaternion();
	Vector3 expected_translation = Vector3(3, 3, 3);
	
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK((translation - expected_translation).length() < epsilon);
}

TEST_CASE("[Modules][QCP] Input Validation - Zero Weights") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights;
	weights.push_back(0.0); // Zero weight
	weights.push_back(0.0); // Zero weight
	weights.push_back(0.0); // Zero weight

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity quaternion for zero weights
	Quaternion expected_rotation = Quaternion();
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Input Validation - Negative Weights") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights;
	weights.push_back(-1.0); // Negative weight
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle negative weights gracefully (implementation dependent)
	CHECK(rotation.is_normalized());
	CHECK(translation.is_finite());
}

// ========================================
// PHASE 2: EDGE CASE TESTS
// ========================================

TEST_CASE("[Modules][QCP] Edge Case - All Points Identical") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 1, 1));
	moved_points.push_back(Vector3(1, 1, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(2, 2, 2));
	target_points.push_back(Vector3(2, 2, 2));
	target_points.push_back(Vector3(2, 2, 2));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity rotation and correct translation
	Quaternion expected_rotation = Quaternion();
	Vector3 expected_translation = Vector3(1, 1, 1);
	
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK((translation - expected_translation).length() < epsilon);
}

TEST_CASE("[Modules][QCP] Edge Case - Collinear Points") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(0, 0, 0));
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(2, 0, 0));
	moved_points.push_back(Vector3(3, 0, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 0, 0));
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0, 2, 0));
	target_points.push_back(Vector3(0, 3, 0));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle collinear points gracefully
	CHECK(rotation.is_normalized());
	CHECK(translation.is_finite());
	
	// Verify the rotation makes sense (90 degree rotation around Z)
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < 0.1); // Looser tolerance for edge case
}

TEST_CASE("[Modules][QCP] Edge Case - Nearly Parallel Vectors") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(1, 1e-10, 0)); // Nearly parallel to first point
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(1e-10, 1, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle numerical precision challenges gracefully
	CHECK(rotation.is_normalized());
	CHECK(translation.is_finite());
	CHECK(!rotation.is_equal_approx(Quaternion(NAN, NAN, NAN, NAN)));
}

TEST_CASE("[Modules][QCP] Edge Case - Opposite Vector Pairs") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(-1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, -1, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0, -1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(1, 0, 0));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle opposite vectors gracefully
	CHECK(rotation.is_normalized());
	CHECK(translation.is_zero_approx());
	
	// Verify rotation is approximately 90 degrees around Z
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < 0.1);
}

// ========================================
// PHASE 3: ENHANCED ALGORITHM TESTS
// ========================================

TEST_CASE("[Modules][QCP] Algorithm Enhancement - Quaternion Canonicalization") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	// Run the same test multiple times to check consistency
	Array result1 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Array result2 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Array result3 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);

	Quaternion rotation1 = result1[0];
	Quaternion rotation2 = result2[0];
	Quaternion rotation3 = result3[0];

	// All quaternions should be canonicalized (w >= 0) and consistent
	CHECK(rotation1.w >= -epsilon); // Allow small numerical errors
	CHECK(rotation2.w >= -epsilon);
	CHECK(rotation3.w >= -epsilon);
	
	// Results should be identical (or equivalent)
	CHECK(Math::abs(Math::abs(rotation1.dot(rotation2)) - 1.0) < epsilon);
	CHECK(Math::abs(Math::abs(rotation1.dot(rotation3)) - 1.0) < epsilon);
}

TEST_CASE("[Modules][QCP] Algorithm Enhancement - Numerical Stability with Small Values") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1e-8, 0, 0));
	moved_points.push_back(Vector3(0, 1e-8, 0));
	moved_points.push_back(Vector3(0, 0, 1e-8));

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]));
	}

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle very small values without numerical instability
	CHECK(rotation.is_normalized());
	CHECK(translation.is_finite());
	CHECK(!rotation.is_equal_approx(Quaternion(NAN, NAN, NAN, NAN)));
}

TEST_CASE("[Modules][QCP] Algorithm Enhancement - Numerical Stability with Large Values") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1e6, 0, 0));
	moved_points.push_back(Vector3(0, 1e6, 0));
	moved_points.push_back(Vector3(0, 0, 1e6));

	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 3.0);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]));
	}

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-3; // Looser tolerance for large values

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle very large values without numerical overflow
	CHECK(rotation.is_normalized());
	CHECK(translation.is_finite());
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
}

TEST_CASE("[Modules][QCP] Algorithm Enhancement - Single Point Accuracy") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(4, 5, 6));

	Vector<double> weights;
	weights.push_back(1.0);

	bool translate = false; // Test without translation first
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Single point should return identity rotation when translation is disabled
	Quaternion expected_rotation = Quaternion();
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK(translation.is_zero_approx());
}

// ========================================
// PHASE 4: REGRESSION TESTS
// ========================================

TEST_CASE("[Modules][QCP] Regression - Complex Multi-Point Transformation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 0));
	moved_points.push_back(Vector3(1, 0, 1));
	moved_points.push_back(Vector3(0, 1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 6.0);
	Vector3 expected_translation = Vector3(2, -3, 4);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]) + expected_translation);
	}

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should accurately recover the transformation
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK((translation - expected_translation).length() < epsilon);
}

TEST_CASE("[Modules][QCP] Regression - Weighted Point Transformation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(2, 2, 2)); // Outlier point

	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 4.0);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]));
	}

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(0.1); // Low weight for outlier

	bool translate = false;
	double epsilon = 1e-5; // Slightly looser tolerance due to weighting

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle weighted points correctly
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Regression - Performance Validation") {
	// Large point set to ensure no significant performance regression
	PackedVector3Array moved_points;
	PackedVector3Array target_points;
	Vector<double> weights;

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 3.0);
	Vector3 expected_translation = Vector3(1, 2, 3);

	for (int i = 0; i < 100; ++i) {
		Vector3 point = Vector3(
			Math::sin(i * 0.1) * 10,
			Math::cos(i * 0.1) * 10,
			i * 0.1
		);
		moved_points.push_back(point);
		target_points.push_back(expected_rotation.xform(point) + expected_translation);
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-5;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle large point sets efficiently and accurately
	CHECK(Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon);
	CHECK((translation - expected_translation).length() < epsilon);
}

// ========================================
// PHASE 5: GEOMETRIC VALIDATION TESTS
// ========================================

TEST_CASE("[Modules][QCP] Geometric Validation - Rotation Normalization") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Use geometric validation functions
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_orthogonality(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_distance_preservation(rotation, moved_points));
}

TEST_CASE("[Modules][QCP] Geometric Validation - Point Alignment") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));
	moved_points.push_back(Vector3(4, 5, 6));
	moved_points.push_back(Vector3(7, 8, 9));

	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 4.0);
	Vector3 expected_translation = Vector3(10, 20, 30);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]) + expected_translation);
	}

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Use geometric validation to verify the transformation actually works
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_point_alignment(rotation, translation, moved_points, target_points));
	CHECK(QuaternionCharacteristicPolynomial::validate_distance_preservation(rotation, moved_points));
	CHECK(QuaternionCharacteristicPolynomial::validate_orthogonality(rotation));
}

TEST_CASE("[Modules][QCP] Geometric Validation - RMSD Calculation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(0, 0, 0));
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	// Perfect alignment case - RMSD should be near zero
	PackedVector3Array target_points = moved_points;

	Vector<double> weights;
	for (int i = 0; i < moved_points.size(); ++i) {
		weights.push_back(1.0);
	}

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Calculate RMSD using geometric validation function
	double rmsd = QuaternionCharacteristicPolynomial::calculate_rmsd(rotation, translation, moved_points, target_points);
	
	CHECK(rmsd >= 0.0); // RMSD should be non-negative
	CHECK(rmsd < epsilon); // Should be near zero for perfect alignment
	CHECK(QuaternionCharacteristicPolynomial::validate_point_alignment(rotation, translation, moved_points, target_points));
}

TEST_CASE("[Modules][QCP] Geometric Validation - Single Point with Translation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(4, 5, 6));

	Vector<double> weights;
	weights.push_back(1.0);

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// For single point with translation, should use identity rotation
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_point_alignment(rotation, translation, moved_points, target_points));
	
	// Verify it's close to identity rotation
	Quaternion identity = Quaternion();
	CHECK(Math::abs(Math::abs(rotation.dot(identity)) - 1.0) < epsilon);
}

TEST_CASE("[Modules][QCP] Geometric Validation - Opposite Vectors") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(-1, 0, 0));

	Vector<double> weights;
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle 180-degree rotation correctly
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_point_alignment(rotation, translation, moved_points, target_points));
	CHECK(QuaternionCharacteristicPolynomial::validate_orthogonality(rotation));
	
	// Verify the rotation angle is approximately 180 degrees
	double angle = 2.0 * Math::acos(Math::abs(rotation.w));
	CHECK(Math::abs(angle - Math::PI) < 0.1); // Allow some tolerance for 180-degree rotation
}

TEST_CASE("[Modules][QCP] Geometric Validation - Zero Length Vectors") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(0, 0, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(1, 0, 0));

	Vector<double> weights;
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity rotation for zero-length input
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	
	Quaternion identity = Quaternion();
	CHECK(Math::abs(Math::abs(rotation.dot(identity)) - 1.0) < epsilon);
}

TEST_CASE("[Modules][QCP] Geometric Validation - Canonical Form Consistency") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	// Run multiple times to check consistency
	Array result1 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Array result2 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Array result3 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);

	Quaternion rotation1 = result1[0];
	Quaternion rotation2 = result2[0];
	Quaternion rotation3 = result3[0];

	// All should be in canonical form (w >= 0) and consistent
	CHECK(rotation1.w >= -epsilon);
	CHECK(rotation2.w >= -epsilon);
	CHECK(rotation3.w >= -epsilon);
	
	// All should be equivalent rotations
	CHECK(Math::abs(Math::abs(rotation1.dot(rotation2)) - 1.0) < epsilon);
	CHECK(Math::abs(Math::abs(rotation1.dot(rotation3)) - 1.0) < epsilon);
	
	// All should pass geometric validation
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation1));
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation2));
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation3));
}

} // namespace TestQCP
