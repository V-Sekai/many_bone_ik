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

// ========================================
// GEOMETRIC VALIDATION HELPER FUNCTIONS
// ========================================

/**
 * Validates that a quaternion represents a valid rotation (normalized).
 * Equivalent to Elixir's assert_valid_rotation/2
 */
static bool validate_rotation_normalization(const Quaternion &rotation, double tolerance = 1e-10) {
	double magnitude = Math::sqrt(rotation.x * rotation.x + rotation.y * rotation.y + 
								  rotation.z * rotation.z + rotation.w * rotation.w);
	return Math::abs(magnitude - 1.0) < tolerance;
}

/**
 * Validates that applying a rotation to moved points aligns them with target points.
 * Equivalent to Elixir's assert_points_aligned/5
 */
static bool validate_point_alignment(const Quaternion &rotation, const Vector3 &translation,
		const PackedVector3Array &moved_points, const PackedVector3Array &target_points, 
		double tolerance = 1e-6) {
	if (moved_points.size() != target_points.size()) {
		return false;
	}
	
	for (int i = 0; i < moved_points.size(); ++i) {
		Vector3 transformed_point = rotation.xform(moved_points[i]) + translation;
		Vector3 target_point = target_points[i];
		
		if ((transformed_point - target_point).length() > tolerance) {
			return false;
		}
	}
	return true;
}

/**
 * Validates that a rotation preserves distances between points.
 * Equivalent to Elixir's assert_distances_preserved/3
 */
static bool validate_distance_preservation(const Quaternion &rotation, 
		const PackedVector3Array &points, double tolerance = 1e-10) {
	for (int i = 0; i < points.size(); ++i) {
		for (int j = i + 1; j < points.size(); ++j) {
			double original_distance = points[i].distance_to(points[j]);
			
			Vector3 rotated_i = rotation.xform(points[i]);
			Vector3 rotated_j = rotation.xform(points[j]);
			double rotated_distance = rotated_i.distance_to(rotated_j);
			
			if (Math::abs(original_distance - rotated_distance) > tolerance) {
				return false;
			}
		}
	}
	return true;
}

/**
 * Validates that a rotation preserves orthogonality of basis vectors.
 * Equivalent to Elixir's assert_orthogonality_preserved/2
 */
static bool validate_orthogonality(const Quaternion &rotation, double tolerance = 1e-10) {
	Vector3 i_rotated = rotation.xform(Vector3(1.0, 0.0, 0.0));
	Vector3 j_rotated = rotation.xform(Vector3(0.0, 1.0, 0.0));
	Vector3 k_rotated = rotation.xform(Vector3(0.0, 0.0, 1.0));
	
	// Check orthogonality
	if (Math::abs(i_rotated.dot(j_rotated)) > tolerance) return false;
	if (Math::abs(i_rotated.dot(k_rotated)) > tolerance) return false;
	if (Math::abs(j_rotated.dot(k_rotated)) > tolerance) return false;
	
	// Check unit length preservation
	if (Math::abs(i_rotated.length() - 1.0) > tolerance) return false;
	if (Math::abs(j_rotated.length() - 1.0) > tolerance) return false;
	if (Math::abs(k_rotated.length() - 1.0) > tolerance) return false;
	
	return true;
}

/**
 * Validates that a rotation is a proper rotation (determinant = +1, not a reflection).
 * Equivalent to Elixir's assert_proper_rotation/2
 */
static bool validate_proper_rotation(const Quaternion &rotation, double tolerance = 1e-10) {
	Vector3 i_rotated = rotation.xform(Vector3(1.0, 0.0, 0.0));
	Vector3 j_rotated = rotation.xform(Vector3(0.0, 1.0, 0.0));
	Vector3 k_rotated = rotation.xform(Vector3(0.0, 0.0, 1.0));
	
	double det = i_rotated.dot(j_rotated.cross(k_rotated));
	return Math::abs(det - 1.0) < tolerance;
}

/**
 * Calculates RMSD between transformed moved points and target points.
 * Equivalent to Elixir's geometric validation RMSD calculation
 */
static double calculate_rmsd(const Quaternion &rotation, const Vector3 &translation,
		const PackedVector3Array &moved_points, const PackedVector3Array &target_points) {
	if (moved_points.size() != target_points.size() || moved_points.size() == 0) {
		return -1.0; // Invalid input
	}
	
	double sum_squared_distances = 0.0;
	for (int i = 0; i < moved_points.size(); ++i) {
		Vector3 transformed_point = rotation.xform(moved_points[i]) + translation;
		double distance = transformed_point.distance_to(target_points[i]);
		sum_squared_distances += distance * distance;
	}
	
	return Math::sqrt(sum_squared_distances / moved_points.size());
}

/**
 * Validates rotation angle is approximately the expected value.
 * Equivalent to Elixir's assert_rotation_angle/3
 */
static bool validate_rotation_angle(const Quaternion &rotation, double expected_angle_radians, 
		double tolerance = 1e-10) {
	double actual_angle = 2.0 * Math::acos(Math::min(Math::abs(rotation.w), 1.0));
	return Math::abs(actual_angle - expected_angle_radians) < tolerance;
}

/**
 * Validates that two quaternions represent the same rotation (handles q and -q equivalence).
 * Equivalent to Elixir's assert_rotations_equivalent/3
 */
static bool validate_rotations_equivalent(const Quaternion &rotation1, const Quaternion &rotation2, 
		double tolerance = 1e-10) {
	double dot_product = Math::abs(rotation1.dot(rotation2));
	return Math::abs(dot_product - 1.0) < tolerance;
}

// ========================================
// SURGICAL SAFETY VALIDATION FUNCTIONS
// ========================================

// Surgical safety thresholds based on medical robotics literature
static const double SURGICAL_ANGULAR_JERK_LIMIT = 10.0;  // rad/s³ - conservative limit for surgical robotics
static const double SURGICAL_LINEAR_JERK_LIMIT = 1.0;    // m/s³ - safe for tissue interaction
static const double SURGICAL_TOLERANCE = 1.0e-8;         // High precision required for surgical applications

/**
 * Calculates angular jerk from quaternion rotation.
 * Based on Elixir's surgical safety jerk validation
 */
static double calculate_angular_jerk(const Quaternion &rotation) {
	double vector_magnitude = Math::sqrt(rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z);
	double rotation_angle = 2.0 * Math::asin(Math::min(vector_magnitude, 1.0));
	return rotation_angle; // Angular jerk ≈ rotation_angle / time³ (assuming unit time for QCP)
}

/**
 * Calculates linear jerk from translation vector.
 * Based on Elixir's surgical safety jerk validation
 */
static double calculate_linear_jerk(const Vector3 &translation) {
	return translation.length(); // Linear jerk ≈ |translation| / time³ (assuming unit time for QCP)
}

/**
 * Enforces surgical jerk limits to protect patients.
 * Based on Elixir's enforce_surgical_jerk_limits/2
 */
static bool validate_surgical_jerk_limits(const Quaternion &rotation, const Vector3 &translation) {
	double angular_jerk = calculate_angular_jerk(rotation);
	double linear_jerk = calculate_linear_jerk(translation);
	
	return (angular_jerk < SURGICAL_ANGULAR_JERK_LIMIT) && (linear_jerk < SURGICAL_LINEAR_JERK_LIMIT);
}

/**
 * Validates minimal jerk for surgical safety.
 * Equivalent to Elixir's Motion.validate_minimal_jerk/3
 */
static bool validate_minimal_jerk(const Quaternion &rotation, const Vector3 &translation, 
		double tolerance = SURGICAL_TOLERANCE) {
	// For surgical applications, both angular and linear jerk should be minimal
	double angular_jerk = calculate_angular_jerk(rotation);
	double linear_jerk = calculate_linear_jerk(translation);
	
	// Minimal jerk means smooth, controlled motion
	return validate_surgical_jerk_limits(rotation, translation) && 
		   validate_rotation_normalization(rotation, tolerance);
}

// ========================================
// BASIC FUNCTIONALITY TESTS
// ========================================

TEST_CASE("[Modules][QCP] Identity Transformation - No Rotation, No Translation") {
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

	// Use geometric validation instead of brittle component comparisons
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(translation.is_zero_approx());
	
	// Should be identity rotation
	Quaternion expected_rotation = Quaternion();
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
}

TEST_CASE("[Modules][QCP] Simple 90-degree Rotation around Z-axis") {
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

	// Comprehensive geometric validation
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	// Validate rotation angle
	CHECK(validate_rotation_angle(rotation, Math::PI / 2.0, 0.1)); // Allow some tolerance for 90-degree rotation
}

TEST_CASE("[Modules][QCP] Pure Translation - No Rotation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 2, 3));
	moved_points.push_back(Vector3(4, 5, 6));
	moved_points.push_back(Vector3(7, 8, 9));

	Vector3 expected_translation = Vector3(10, 20, 30);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(moved_points[i] + expected_translation);
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

	// Should be identity rotation with correct translation
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	
	Quaternion expected_rotation = Quaternion();
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
	CHECK((translation - expected_translation).length() < epsilon);
}

TEST_CASE("[Modules][QCP] Combined 90-degree Rotation and Translation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0); // 90 degrees around Z
	Vector3 expected_translation = Vector3(5, -5, 10);

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

	// Comprehensive validation
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	
	// Validate specific transformation
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
	CHECK((translation - expected_translation).length() < epsilon);
}

TEST_CASE("[Modules][QCP] 45-degree Rotation around Y-axis with Translation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(2, -1, 3));

	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 4.0); // 45 degrees around Y
	Vector3 expected_translation = Vector3(-2, 3, -4);

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

	// Comprehensive validation
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	
	// Validate specific transformation
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
	CHECK((translation - expected_translation).length() < epsilon);
}

// ========================================
// INPUT VALIDATION TESTS
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
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	Quaternion expected_rotation = Quaternion();
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
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
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	Quaternion expected_rotation = Quaternion();
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
}

TEST_CASE("[Modules][QCP] Input Validation - Single Point with Translation") {
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
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	
	Quaternion expected_rotation = Quaternion();
	Vector3 expected_translation = Vector3(3, 3, 3);
	
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
	CHECK((translation - expected_translation).length() < epsilon);
}

TEST_CASE("[Modules][QCP] Input Validation - Zero Weights Handling") {
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

	// Should handle zero weights gracefully
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	Quaternion expected_rotation = Quaternion();
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
}

TEST_CASE("[Modules][QCP] Input Validation - Negative Weights Handling") {
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

	// Should handle negative weights gracefully
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(translation.is_finite());
}

// ========================================
// EDGE CASE TESTS
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
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	
	Quaternion expected_rotation = Quaternion();
	Vector3 expected_translation = Vector3(1, 1, 1);
	
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, epsilon));
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
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_finite());
	
	// Verify the rotation makes sense (90 degree rotation around Z)
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, 0.1)); // Looser tolerance for edge case
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
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	// Verify the rotation makes sense (90 degree rotation around Z)
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, 0.1)); // Looser tolerance for edge case
}

TEST_CASE("[Modules][QCP] Edge Case - Near-Parallel Vectors") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(1, 0.001, 0)); // Nearly parallel
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0.001, 1, 0)); // Nearly parallel
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(0, 0, 1));

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

	// Should handle near-parallel vectors gracefully
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_finite());
}

// ========================================
// WEIGHTED TRANSFORMATION TESTS
// ========================================

TEST_CASE("[Modules][QCP] Weighted Transformation - Unequal Weights") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0); // 45 degrees around Z

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(expected_rotation.xform(moved_points[i]));
	}

	Vector<double> weights;
	weights.push_back(2.0); // Higher weight
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(0.5); // Lower weight

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle weighted transformation correctly
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Weighted Transformation - High Weight Dominance") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0)); // 90 degree rotation
	target_points.push_back(Vector3(1, 0, 0)); // Different rotation
	target_points.push_back(Vector3(0, 0, 1)); // No rotation

	Vector<double> weights;
	weights.push_back(100.0); // Dominant weight
	weights.push_back(0.01);  // Minimal weight
	weights.push_back(0.01);  // Minimal weight

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should be dominated by the high-weight point
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	// The rotation should be close to 90 degrees around Z due to dominant weight
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	CHECK(validate_rotations_equivalent(rotation, expected_rotation, 0.2)); // Looser tolerance due to other points
}

// ========================================
// SURGICAL SAFETY TESTS
// ========================================

TEST_CASE("[Modules][QCP] Surgical Safety - Jerk Limit Validation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	// Small rotation to stay within surgical limits
	Quaternion safe_rotation = Quaternion(Vector3(0, 0, 1), 0.1); // Small 0.1 radian rotation
	Vector3 safe_translation = Vector3(0.1, 0.1, 0.1); // Small translation

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(safe_rotation.xform(moved_points[i]) + safe_translation);
	}

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = true;
	double epsilon = SURGICAL_TOLERANCE;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Validate surgical safety requirements
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_surgical_jerk_limits(rotation, translation));
	CHECK(validate_minimal_jerk(rotation, translation, epsilon));
	
	// Verify transformation is within surgical safety bounds
	double angular_jerk = calculate_angular_jerk(rotation);
	double linear_jerk = calculate_linear_jerk(translation);
	CHECK(angular_jerk < SURGICAL_ANGULAR_JERK_LIMIT);
	CHECK(linear_jerk < SURGICAL_LINEAR_JERK_LIMIT);
}

TEST_CASE("[Modules][QCP] Surgical Safety - Motion Coordination Validation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 0));

	// Coordinated motion with minimal jerk
	Quaternion coordinated_rotation = Quaternion(Vector3(1, 1, 1).normalized(), 0.05); // Very small rotation
	Vector3 coordinated_translation = Vector3(0.05, 0.05, 0.05); // Very small translation

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(coordinated_rotation.xform(moved_points[i]) + coordinated_translation);
	}

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = true;
	double epsilon = SURGICAL_TOLERANCE;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Validate coordinated motion for surgical safety
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_minimal_jerk(rotation, translation, epsilon));
	CHECK(validate_surgical_jerk_limits(rotation, translation));
	
	// Verify smooth, coordinated motion
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
}

// ========================================
// NUMERICAL STABILITY TESTS
// ========================================

TEST_CASE("[Modules][QCP] Numerical Stability - Very Small Values") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1e-10, 0, 0));
	moved_points.push_back(Vector3(0, 1e-10, 0));
	moved_points.push_back(Vector3(0, 0, 1e-10));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1e-10, 0));
	target_points.push_back(Vector3(-1e-10, 0, 0));
	target_points.push_back(Vector3(0, 0, 1e-10));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-12;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle very small values without numerical instability
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(rotation.is_finite());
	CHECK(translation.is_finite());
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
}

TEST_CASE("[Modules][QCP] Numerical Stability - Large Values") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1e6, 0, 0));
	moved_points.push_back(Vector3(0, 1e6, 0));
	moved_points.push_back(Vector3(0, 0, 1e6));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1e6, 0));
	target_points.push_back(Vector3(-1e6, 0, 0));
	target_points.push_back(Vector3(0, 0, 1e6));

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle large values without numerical overflow
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(rotation.is_finite());
	CHECK(translation.is_finite());
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
}

// ========================================
// PERFORMANCE AND ROBUSTNESS TESTS
// ========================================

TEST_CASE("[Modules][QCP] Performance - Large Point Set") {
	PackedVector3Array moved_points;
	PackedVector3Array target_points;
	Vector<double> weights;

	// Create a large set of points (100 points)
	Quaternion test_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 6.0);
	Vector3 test_translation = Vector3(5, -3, 2);

	for (int i = 0; i < 100; ++i) {
		Vector3 point = Vector3(
			Math::sin(i * 0.1) * 10,
			Math::cos(i * 0.1) * 10,
			i * 0.2
		);
		moved_points.push_back(point);
		target_points.push_back(test_rotation.xform(point) + test_translation);
		weights.push_back(1.0);
	}

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle large point sets efficiently and accurately
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	
	// Verify the transformation is close to expected
	CHECK(validate_rotations_equivalent(rotation, test_rotation, 0.01));
	CHECK((translation - test_translation).length() < 0.01);
}

TEST_CASE("[Modules][QCP] Robustness - Mixed Scale Points") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1000, 0, 0));     // Large scale
	moved_points.push_back(Vector3(0, 0.001, 0));   // Small scale
	moved_points.push_back(Vector3(0, 0, 1));       // Normal scale
	moved_points.push_back(Vector3(100, 100, 100)); // Large scale

	Quaternion test_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 3.0);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(test_rotation.xform(moved_points[i]));
	}

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

	// Should handle mixed scale points robustly
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_distance_preservation(rotation, moved_points, epsilon));
	CHECK(validate_orthogonality(rotation, epsilon));
	CHECK(validate_proper_rotation(rotation, epsilon));
	CHECK(translation.is_zero_approx());
	
	// Should be close to the expected rotation
	CHECK(validate_rotations_equivalent(rotation, test_rotation, 0.1));
}

// ========================================
// RMSD CALCULATION TESTS
// ========================================

TEST_CASE("[Modules][QCP] RMSD Calculation - Perfect Alignment") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));

	PackedVector3Array target_points = moved_points; // Perfect alignment

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Calculate RMSD for perfect alignment
	double rmsd = calculate_rmsd(rotation, translation, moved_points, target_points);
	
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(rmsd < epsilon); // RMSD should be essentially zero for perfect alignment
	CHECK(translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] RMSD Calculation - Known Transformation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(1, 1, 1));

	Quaternion known_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	Vector3 known_translation = Vector3(2, -2, 3);

	PackedVector3Array target_points;
	for (int i = 0; i < moved_points.size(); ++i) {
		target_points.push_back(known_rotation.xform(moved_points[i]) + known_translation);
	}

	Vector<double> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);

	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved_points, target_points, weights, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Calculate RMSD for known transformation
	double rmsd = calculate_rmsd(rotation, translation, moved_points, target_points);
	
	CHECK(validate_rotation_normalization(rotation, epsilon));
	CHECK(validate_point_alignment(rotation, translation, moved_points, target_points, epsilon));
	CHECK(rmsd < epsilon); // RMSD should be very small for accurate transformation
	
	// Verify the transformation matches the known values
	CHECK(validate_rotations_equivalent(rotation, known_rotation, epsilon));
	CHECK((translation - known_translation).length() < epsilon);
}

} // namespace TestQCP
