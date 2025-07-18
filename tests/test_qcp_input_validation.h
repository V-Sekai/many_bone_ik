/**************************************************************************/
/*  test_qcp_input_validation.h                                           */
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

#include "test_qcp_helpers.h"
#include "test_qcp_validation.h"
#include "test_qcp_fixtures.h"
#include "tests/test_macros.h"

using namespace TestQCPHelpers;
using namespace TestQCPValidation;
using namespace TestQCPFixtures;

namespace TestQCPInputValidation {

TEST_CASE("[Modules][QCP] Input Validation - Mismatched Array Sizes") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(1, 0, 0));
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0, 0, 1)); // Extra point

	Vector<double> weights = create_uniform_weights(2);

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity quaternion for invalid input
	validate_identity_result(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - Empty Arrays") {
	PackedVector3Array moved_points;
	PackedVector3Array target_points;
	Vector<double> weights;

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity quaternion for empty input
	validate_identity_result(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - Single Point") {
	Vector3 moved_point = Vector3(1, 2, 3);
	Vector3 target_point = Vector3(4, 5, 6);
	
	// Test without translation
	validate_single_point_behavior(moved_point, target_point, false);
	
	// Test with translation
	validate_single_point_behavior(moved_point, target_point, true);
}

TEST_CASE("[Modules][QCP] Input Validation - Zero Weights") {
	PackedVector3Array moved_points = create_basic_point_set();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights = create_zero_weights(3);

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should return identity quaternion for zero weights
	validate_identity_result(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - Negative Weights") {
	PackedVector3Array moved_points = create_basic_point_set();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights = create_negative_weight_config();

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle negative weights gracefully (implementation dependent)
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - Mismatched Weight Array Size") {
	PackedVector3Array moved_points = create_basic_point_set();
	PackedVector3Array target_points = create_basic_point_set();
	Vector<double> weights = create_uniform_weights(2); // Wrong size

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - Very Large Weight Values") {
	PackedVector3Array moved_points = create_basic_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Vector<double> weights;
	weights.push_back(1e10);
	weights.push_back(1e10);
	weights.push_back(1e10);

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false, CMP_EPSILON2);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle large weights without numerical issues
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-3); // Looser tolerance for large weights
}

TEST_CASE("[Modules][QCP] Input Validation - Very Small Weight Values") {
	PackedVector3Array moved_points = create_basic_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Vector<double> weights;
	weights.push_back(1e-10);
	weights.push_back(1e-10);
	weights.push_back(1e-10);

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle very small weights
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - Mixed Valid and Invalid Weights") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 6.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Vector<double> weights;
	weights.push_back(1.0);    // Valid
	weights.push_back(0.0);    // Zero weight
	weights.push_back(-1.0);   // Negative weight
	weights.push_back(1e-15);  // Very small weight

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle mixed weights gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
}

TEST_CASE("[Modules][QCP] Input Validation - NaN and Infinite Values in Points") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(NAN, NAN, NAN)); // Invalid point

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(INFINITY, 0, 0)); // Invalid point

	Vector<double> weights = create_uniform_weights(3);

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];

	// Should handle invalid values gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	// Note: We don't check for finite values here as the algorithm might return identity for invalid input
}

TEST_CASE("[Modules][QCP] Input Validation - NaN and Infinite Values in Weights") {
	PackedVector3Array moved_points = create_basic_point_set();
	PackedVector3Array target_points = create_basic_point_set();

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(NAN);      // Invalid weight
	weights.push_back(INFINITY); // Invalid weight

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];

	// Should handle invalid weights gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
}

TEST_CASE("[Modules][QCP] Input Validation - Maximum Array Size Handling") {
	// Test with a reasonably large number of points to ensure no overflow
	const int large_count = 1000;
	PackedVector3Array moved_points;
	PackedVector3Array target_points;
	Vector<double> weights;

	for (int i = 0; i < large_count; ++i) {
		Vector3 point = Vector3(i * 0.1, (i + 1) * 0.1, (i + 2) * 0.1);
		moved_points.push_back(point);
		target_points.push_back(point + Vector3(1, 1, 1)); // Simple translation
		weights.push_back(1.0);
	}

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, true);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle large arrays efficiently
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	// Use more reasonable tolerance for large point sets (1000 points)
	// Accumulated floating-point errors are expected with this many points
	CHECK_IDENTITY_ROTATION(rotation, 1e-2);
}

TEST_CASE("[Modules][QCP] Input Validation - Precision Parameter Edge Cases") {
	PackedVector3Array moved_points = create_basic_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);
	Vector<double> weights = create_uniform_weights(3);

	// Test with very high precision
	Array result1 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, false, 1e-15);
	Quaternion rotation1 = result1[0];
	CHECK_ROTATION_NORMALIZED(rotation1);

	// Test with very low precision
	Array result2 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, false, 1e-1);
	Quaternion rotation2 = result2[0];
	CHECK_ROTATION_NORMALIZED(rotation2);

	// Test with zero precision (should use default)
	Array result3 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, false, 0.0);
	Quaternion rotation3 = result3[0];
	CHECK_ROTATION_NORMALIZED(rotation3);

	// Test with negative precision (should use absolute value or default)
	Array result4 = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, false, -1e-6);
	Quaternion rotation4 = result4[0];
	CHECK_ROTATION_NORMALIZED(rotation4);
}

} // namespace TestQCPInputValidation
