/**************************************************************************/
/*  test_qcp_advanced.h                                                   */
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

#include "test_qcp_fixtures.h"
#include "test_qcp_helpers.h"
#include "test_qcp_validation.h"
#include "tests/test_macros.h"


using namespace TestQCPHelpers;
using namespace TestQCPValidation;
using namespace TestQCPFixtures;

namespace TestQCPAdvanced {

TEST_CASE("[Modules][QCP] Algorithm Enhancement - Quaternion Canonicalization") {
	PackedVector3Array moved_points = create_basic_point_set();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Vector<double> weights = create_uniform_weights(3);
	validate_canonical_form_consistency(moved_points, target_points, weights, false);
}

TEST_CASE("[Modules][QCP] Algorithm Enhancement - Single Point Accuracy") {
	Vector3 moved_point = Vector3(1, 2, 3);
	Vector3 target_point = Vector3(4, 5, 6);

	// Test without translation first
	validate_single_point_behavior(moved_point, target_point, false);
}

TEST_CASE("[Modules][QCP] Regression - Complex Multi-Point Transformation") {
	PackedVector3Array moved_points = create_complex_multi_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 6.0);
	Vector3 expected_translation = Vector3(2, -3, 4);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should accurately recover the transformation
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	CHECK((translation - expected_translation).length() < 1e-6);
}

TEST_CASE("[Modules][QCP] Regression - Weighted Point Transformation") {
	PackedVector3Array moved_points = create_extended_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Vector<double> weights = create_weighted_with_outlier(4);

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle weighted points correctly
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-5);
	CHECK_TRANSLATION_ZERO(translation);
}

TEST_CASE("[Modules][QCP] Regression - Performance Validation") {
	// Large point set to ensure no significant performance regression
	PackedVector3Array moved_points = create_performance_test_points();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 3.0);
	Vector3 expected_translation = Vector3(1, 2, 3);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true, 1e-5);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle large point sets efficiently and accurately
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-5);
	CHECK((translation - expected_translation).length() < 1e-5);
}

TEST_CASE("[Modules][QCP] Geometric Validation - Rotation Normalization") {
	PackedVector3Array moved_points = create_basic_point_set();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Array result = compute_qcp_transformation(moved_points, target_points, false, CMP_EPSILON2);
	Quaternion rotation = result[0];

	// Use geometric validation functions
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_orthogonality(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_distance_preservation(rotation, moved_points));
}

TEST_CASE("[Modules][QCP] Geometric Validation - Point Alignment") {
	PackedVector3Array moved_points = create_simple_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 4.0);
	Vector3 expected_translation = Vector3(10, 20, 30);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true, CMP_EPSILON2);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Use geometric validation to verify the transformation actually works
	validate_geometric_properties(rotation, moved_points, target_points, translation);
}

TEST_CASE("[Modules][QCP] Geometric Validation - RMSD Calculation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(0, 0, 0));
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	// Perfect alignment case - RMSD should be near zero
	PackedVector3Array target_points = moved_points;

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Calculate RMSD using geometric validation function
	validate_rmsd_properties(rotation, translation, moved_points, target_points);

	double rmsd = QuaternionCharacteristicPolynomial::calculate_rmsd(rotation, translation, moved_points, target_points);
	CHECK(rmsd < 1e-6); // Should be near zero for perfect alignment
	CHECK_POINT_ALIGNMENT(rotation, translation, moved_points, target_points);
}

TEST_CASE("[Modules][QCP] Geometric Validation - Canonical Form Consistency") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(-1, 0, 0));

	Vector<double> weights = create_uniform_weights(2);
	validate_canonical_form_consistency(moved_points, target_points, weights, false);
}

TEST_CASE("[Modules][QCP] Advanced - Multiple Rotation Axes Validation") {
	PackedVector3Array moved_points = create_extended_point_set();

	// Test rotations around different axes
	Vector3 axes[] = {
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(1, 1, 0).normalized(),
		Vector3(1, 0, 1).normalized(),
		Vector3(0, 1, 1).normalized(),
		Vector3(1, 1, 1).normalized()
	};

	double angles[] = { Math::PI / 6.0, Math::PI / 4.0, Math::PI / 3.0, Math::PI / 2.0 };

	for (int axis_idx = 0; axis_idx < 7; ++axis_idx) {
		for (int angle_idx = 0; angle_idx < 4; ++angle_idx) {
			Quaternion expected_rotation = Quaternion(axes[axis_idx], angles[angle_idx]);
			PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

			Array result = compute_qcp_transformation(moved_points, target_points, false);
			Quaternion rotation = result[0];
			Vector3 translation = result[1];

			CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
			CHECK_TRANSLATION_ZERO(translation);
			validate_geometric_properties(rotation, moved_points, target_points, translation);
		}
	}
}

TEST_CASE("[Modules][QCP] Advanced - Stress Test with Random Transformations") {
	PackedVector3Array moved_points = create_complex_multi_point_set();

	// Test multiple random-like transformations
	struct TestTransform {
		Quaternion rotation;
		Vector3 translation;
	};

	TestTransform transforms[] = {
		{ Quaternion(Vector3(0.267, 0.535, 0.802).normalized(), 0.785), Vector3(1.5, -2.3, 4.7) },
		{ Quaternion(Vector3(-0.408, 0.816, -0.408).normalized(), 1.047), Vector3(-3.2, 1.8, -0.9) },
		{ Quaternion(Vector3(0.577, -0.577, 0.577).normalized(), 1.571), Vector3(0.1, -0.2, 0.3) },
		{ Quaternion(Vector3(0.707, 0.0, 0.707).normalized(), 2.094), Vector3(10.0, -5.0, 2.5) }
	};

	for (int i = 0; i < 4; ++i) {
		const TestTransform &transform = transforms[i];
		PackedVector3Array target_points = apply_transformation(moved_points, transform.rotation, transform.translation);

		Array result = compute_qcp_transformation(moved_points, target_points, true);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];

		CHECK_ROTATION_EQUIVALENT(rotation, transform.rotation, 1e-6);
		CHECK((translation - transform.translation).length() < 1e-6);
		validate_transformation_accuracy(rotation, translation, moved_points, target_points);
	}
}

TEST_CASE("[Modules][QCP] Advanced - Weighted Transformation Accuracy") {
	PackedVector3Array moved_points = create_complex_multi_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 5.0);
	Vector3 expected_translation = Vector3(1.5, -2.5, 3.5);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	// Test different weight distributions
	Vector<Vector<double>> weight_configs;

	// Uniform weights
	weight_configs.push_back(create_uniform_weights(6, 1.0));

	// Decreasing weights
	Vector<double> decreasing_weights;
	for (int i = 0; i < 6; ++i) {
		decreasing_weights.push_back(1.0 / (i + 1));
	}
	weight_configs.push_back(decreasing_weights);

	// High weight on first point
	Vector<double> first_high_weights;
	first_high_weights.push_back(10.0);
	for (int i = 1; i < 6; ++i) {
		first_high_weights.push_back(1.0);
	}
	weight_configs.push_back(first_high_weights);

	for (int config_idx = 0; config_idx < weight_configs.size(); ++config_idx) {
		Array result = compute_qcp_transformation_weighted(moved_points, target_points, weight_configs[config_idx], true);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];

		// All weight configurations should produce reasonable results
		CHECK_ROTATION_NORMALIZED(rotation);
		CHECK_FINITE_VALUES(rotation, translation);
		validate_transformation_accuracy(rotation, translation, moved_points, target_points, 1e-5);
	}
}

TEST_CASE("[Modules][QCP] Advanced - Numerical Precision Boundary Testing") {
	PackedVector3Array moved_points = create_basic_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 7.0); // Non-standard angle
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	// Test with different precision requirements
	double precisions[] = { 1e-15, 1e-12, 1e-9, 1e-6, 1e-3 };

	for (int i = 0; i < 5; ++i) {
		Array result = compute_qcp_transformation(moved_points, target_points, false, precisions[i]);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];

		CHECK_ROTATION_NORMALIZED(rotation);
		CHECK_FINITE_VALUES(rotation, translation);

		// Higher precision should give more accurate results
		double tolerance = MAX(precisions[i] * 10.0, 1e-6);
		CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, tolerance);
	}
}

TEST_CASE("[Modules][QCP] Advanced - Memory and Performance Stress Test") {
	// Test with progressively larger point sets
	int sizes[] = { 10, 50, 100, 500, 1000 };

	for (int size_idx = 0; size_idx < 5; ++size_idx) {
		int size = sizes[size_idx];
		PackedVector3Array moved_points = create_performance_test_points(size);

		Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 8.0);
		Vector3 expected_translation = Vector3(0.5, -1.5, 2.0);
		PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

		Array result = compute_qcp_transformation(moved_points, target_points, true, 1e-5);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];

		// Should handle all sizes efficiently
		CHECK_ROTATION_NORMALIZED(rotation);
		CHECK_FINITE_VALUES(rotation, translation);
		CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-4);
		CHECK((translation - expected_translation).length() < 1e-4);
	}
}

TEST_CASE("[Modules][QCP] Advanced - Comprehensive Geometric Validation Suite") {
	PackedVector3Array moved_points = create_extended_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0.6, 0.8, 0.0).normalized(), Math::PI / 3.0);
	Vector3 expected_translation = Vector3(2.5, -1.5, 3.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Comprehensive validation
	CHECK(QuaternionCharacteristicPolynomial::validate_rotation_normalization(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_orthogonality(rotation));
	CHECK(QuaternionCharacteristicPolynomial::validate_point_alignment(rotation, translation, moved_points, target_points));
	CHECK(QuaternionCharacteristicPolynomial::validate_distance_preservation(rotation, moved_points));

	validate_rmsd_properties(rotation, translation, moved_points, target_points);
	validate_transformation_accuracy(rotation, translation, moved_points, target_points);

	// Verify the computed transformation is close to expected
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	CHECK((translation - expected_translation).length() < 1e-6);
}

} // namespace TestQCPAdvanced
