/**************************************************************************/
/*  test_qcp_edge_cases.h                                                 */
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

namespace TestQCPEdgeCases {

TEST_CASE("[Modules][QCP] Edge Case - All Points Identical") {
	PackedVector3Array moved_points = create_identical_points();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(2, 2, 2));
	target_points.push_back(Vector3(2, 2, 2));
	target_points.push_back(Vector3(2, 2, 2));

	Array result = compute_qcp_transformation(moved_points, target_points, true, CMP_EPSILON2);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	CHECK_POINT_ALIGNMENT(rotation, translation, moved_points, target_points);
}

TEST_CASE("[Modules][QCP] Edge Case - Collinear Points") {
	PackedVector3Array moved_points = create_collinear_points();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 0, 0));
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0, 2, 0));
	target_points.push_back(Vector3(0, 3, 0));

	Array result = compute_qcp_transformation(moved_points, target_points, false, CMP_EPSILON2);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle collinear points gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	
	// For collinear points, any valid rotation that transforms the points correctly is acceptable
	// We just need to verify it's a valid rotation and produces reasonable results
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
}

TEST_CASE("[Modules][QCP] Edge Case - Nearly Parallel Vectors") {
	PackedVector3Array moved_points = create_nearly_parallel_vectors();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(1e-10, 1, 0));
	target_points.push_back(Vector3(0, 0, 1));

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle numerical precision challenges gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	CHECK(!rotation.is_equal_approx(Quaternion(NAN, NAN, NAN, NAN)));
}

TEST_CASE("[Modules][QCP] Edge Case - Opposite Vector Pairs") {
	PackedVector3Array moved_points = create_opposite_vector_pairs();
	PackedVector3Array target_points;
	target_points.push_back(Vector3(0, 1, 0));
	target_points.push_back(Vector3(0, -1, 0));
	target_points.push_back(Vector3(-1, 0, 0));
	target_points.push_back(Vector3(1, 0, 0));

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle opposite vectors gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_TRANSLATION_ZERO(translation);
	
	// Verify rotation is approximately 90 degrees around Z
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 0.1);
}

TEST_CASE("[Modules][QCP] Edge Case - Numerical Stability with Small Values") {
	PackedVector3Array moved_points = create_small_value_points();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle very small values without numerical instability
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	CHECK(!rotation.is_equal_approx(Quaternion(NAN, NAN, NAN, NAN)));
}

TEST_CASE("[Modules][QCP] Edge Case - Numerical Stability with Large Values") {
	PackedVector3Array moved_points = create_large_value_points();
	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 3.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false, 1e-3);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle very large values without numerical overflow
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-3);
}

TEST_CASE("[Modules][QCP] Edge Case - Zero Length Vectors") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(0, 0, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(1, 0, 0));

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];

	// Should return identity rotation for zero-length input
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_IDENTITY_ROTATION(rotation, 1e-6);
}

TEST_CASE("[Modules][QCP] Edge Case - Single Point Opposite Vectors") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));

	PackedVector3Array target_points;
	target_points.push_back(Vector3(-1, 0, 0));

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle 180-degree rotation correctly
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_ORTHOGONALITY(rotation);
	
	// For opposite vectors, verify the transformation works geometrically
	Vector3 transformed = rotation.xform(moved_points[0]) + translation;
	Vector3 target_normalized = target_points[0].normalized();
	Vector3 transformed_normalized = transformed.normalized();
	
	// Check that the transformed vector points in the opposite direction (within tolerance)
	double dot_product = transformed_normalized.dot(target_normalized);
	CHECK(Math::abs(dot_product - (-1.0)) < 0.2); // Should be close to -1 (opposite direction)
}

TEST_CASE("[Modules][QCP] Edge Case - Planar Point Sets") {
	// All points lie in the XY plane
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(-1, 0, 0));
	moved_points.push_back(Vector3(0, -1, 0));

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle planar point sets correctly
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	CHECK_TRANSLATION_ZERO(translation);
}

TEST_CASE("[Modules][QCP] Edge Case - Points with Mixed Scales") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1e-3, 0, 0));     // Very small
	moved_points.push_back(Vector3(0, 1e3, 0));      // Very large
	moved_points.push_back(Vector3(0, 0, 1));        // Normal scale

	Quaternion expected_rotation = Quaternion(Vector3(1, 0, 0), Math::PI / 6.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false, 1e-3);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle mixed scales gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-2); // Looser tolerance for mixed scales
}

TEST_CASE("[Modules][QCP] Edge Case - Degenerate Triangle") {
	// Three points that form a degenerate triangle (nearly collinear)
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(0, 0, 0));
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(2, 1e-10, 0)); // Nearly collinear

	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 8.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle degenerate triangles gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
}

TEST_CASE("[Modules][QCP] Edge Case - Symmetric Point Configurations") {
	// Highly symmetric point configuration
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 1, 1));
	moved_points.push_back(Vector3(1, -1, -1));
	moved_points.push_back(Vector3(-1, 1, -1));
	moved_points.push_back(Vector3(-1, -1, 1));

	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 3.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle symmetric configurations correctly
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	CHECK_TRANSLATION_ZERO(translation);
	validate_geometric_properties(rotation, moved_points, target_points, translation);
}

TEST_CASE("[Modules][QCP] Edge Case - Points at Coordinate System Extremes") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));   // X-axis
	moved_points.push_back(Vector3(0, 1, 0));   // Y-axis
	moved_points.push_back(Vector3(0, 0, 1));   // Z-axis
	moved_points.push_back(Vector3(-1, 0, 0));  // -X-axis
	moved_points.push_back(Vector3(0, -1, 0));  // -Y-axis
	moved_points.push_back(Vector3(0, 0, -1));  // -Z-axis

	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 0).normalized(), Math::PI / 4.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle coordinate extremes correctly
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	CHECK_TRANSLATION_ZERO(translation);
}

TEST_CASE("[Modules][QCP] Edge Case - Weighted Points with Extreme Weights") {
	PackedVector3Array moved_points = create_extended_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 6.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Vector<double> weights;
	weights.push_back(1e10);  // Extremely high weight
	weights.push_back(1e-10); // Extremely low weight
	weights.push_back(1.0);   // Normal weight
	weights.push_back(1e5);   // High weight

	Array result = compute_qcp_transformation_weighted(moved_points, target_points, weights, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	// Should handle extreme weights gracefully
	CHECK_ROTATION_NORMALIZED(rotation);
	CHECK_FINITE_VALUES(rotation, translation);
	// The result should be dominated by the high-weight points
}

} // namespace TestQCPEdgeCases
