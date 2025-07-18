/**************************************************************************/
/*  test_ik_open_cone_3d_singularities.h                                  */
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
#include "modules/many_bone_ik/src/ik_kusudama_3d.h"
#include "modules/many_bone_ik/src/ik_open_cone_3d.h"
#include "tests/test_macros.h"

namespace TestIKOpenCone3DSingularities {

// Helper function to check if a vector is finite and normalized
void check_vector_valid(const Vector3 &vec, const String &context = "") {
	CHECK_MESSAGE(vec.is_finite(), vformat("Vector should be finite in %s", context));
	if (!vec.is_zero_approx()) {
		real_t length = vec.length();
		CHECK_MESSAGE(Math::is_equal_approx(length, real_t(1.0)), vformat("Vector should be normalized in %s", context));
	}
}

// Helper function to check if a vector is finite (allows non-normalized)
void check_vector_finite(const Vector3 &vec, const String &context = "") {
	CHECK_MESSAGE(vec.is_finite(), vformat("Vector should be finite in %s", context));
	CHECK_MESSAGE(!Math::is_nan(vec.x), vformat("Vector.x should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(vec.y), vformat("Vector.y should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(vec.z), vformat("Vector.z should not be NaN in %s", context));
}

// Helper function to create a basic kusudama for cone testing
Ref<IKKusudama3D> create_test_kusudama() {
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();
	return kusudama;
}

// Helper function to create a basic cone
Ref<IKLimitCone3D> create_test_cone(Ref<IKKusudama3D> kusudama, const Vector3 &control_point, double radius) {
	Ref<IKLimitCone3D> cone;
	cone.instantiate();
	cone->set_attached_to(kusudama);
	cone->set_control_point(control_point);
	cone->set_radius(radius);
	return cone;
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Parallel Control Points") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create two cones with exactly parallel control points
	Vector3 control_point_a = Vector3(0, 0, 1);
	Vector3 control_point_b = Vector3(0, 0, 1); // Exactly parallel

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, control_point_a, Math::PI / 6);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, control_point_b, Math::PI / 4);

	// This should not crash or produce NaN values
	cone_a->update_tangent_handles(cone_b);

	// Verify tangent centers are valid
	Vector3 tangent_center_1 = cone_a->get_tangent_circle_center_next_1();
	Vector3 tangent_center_2 = cone_a->get_tangent_circle_center_next_2();

	check_vector_finite(tangent_center_1, "parallel control points tangent center 1");
	check_vector_finite(tangent_center_2, "parallel control points tangent center 2");

	// Tangent radius should be finite
	double tangent_radius = cone_a->get_tangent_circle_radius_next();
	CHECK_MESSAGE(Math::is_finite(tangent_radius), "Tangent radius should be finite for parallel control points");
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Anti-Parallel Control Points") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create two cones with exactly anti-parallel control points
	Vector3 control_point_a = Vector3(0, 0, 1);
	Vector3 control_point_b = Vector3(0, 0, -1); // Exactly anti-parallel

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, control_point_a, Math::PI / 6);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, control_point_b, Math::PI / 4);

	// This should not crash or produce NaN values
	cone_a->update_tangent_handles(cone_b);

	// Verify tangent centers are valid
	Vector3 tangent_center_1 = cone_a->get_tangent_circle_center_next_1();
	Vector3 tangent_center_2 = cone_a->get_tangent_circle_center_next_2();

	check_vector_finite(tangent_center_1, "anti-parallel control points tangent center 1");
	check_vector_finite(tangent_center_2, "anti-parallel control points tangent center 2");

	// Tangent radius should be finite
	double tangent_radius = cone_a->get_tangent_circle_radius_next();
	CHECK_MESSAGE(Math::is_finite(tangent_radius), "Tangent radius should be finite for anti-parallel control points");
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Nearly Parallel Control Points") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create two cones with nearly parallel control points (within singularity epsilon)
	Vector3 control_point_a = Vector3(0, 0, 1);
	Vector3 control_point_b = Vector3(1e-7, 0, 1).normalized(); // Nearly parallel

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, control_point_a, Math::PI / 6);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, control_point_b, Math::PI / 4);

	// This should not crash or produce NaN values
	cone_a->update_tangent_handles(cone_b);

	// Verify tangent centers are valid
	Vector3 tangent_center_1 = cone_a->get_tangent_circle_center_next_1();
	Vector3 tangent_center_2 = cone_a->get_tangent_circle_center_next_2();

	check_vector_finite(tangent_center_1, "nearly parallel control points tangent center 1");
	check_vector_finite(tangent_center_2, "nearly parallel control points tangent center 2");

	// Verify the tangent centers are not identical (should be distinct)
	CHECK_MESSAGE(!tangent_center_1.is_equal_approx(tangent_center_2), "Tangent centers should be distinct for nearly parallel control points");
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Zero Length Control Point") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create cone with zero-length control point (should be handled gracefully)
	Vector3 zero_control_point = Vector3(0, 0, 0);

	Ref<IKLimitCone3D> cone = create_test_cone(kusudama, zero_control_point, Math::PI / 6);

	// Verify the control point was normalized to a default value
	Vector3 actual_control_point = cone->get_control_point();
	check_vector_valid(actual_control_point, "zero-length control point normalization");

	// Should default to (0, 1, 0) as per the implementation
	CHECK(actual_control_point.is_equal_approx(Vector3(0, 1, 0)));
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Closest To Cone With Aligned Input") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	Vector3 control_point = Vector3(0, 0, 1);
	Ref<IKLimitCone3D> cone = create_test_cone(kusudama, control_point, Math::PI / 4);

	// Test with input exactly aligned with control point
	Vector3 aligned_input = Vector3(0, 0, 1);
	Vector<double> in_bounds;
	in_bounds.resize(1);
	in_bounds.write[0] = 0;

	Vector3 result = cone->closest_to_cone(aligned_input, &in_bounds);

	// Should return NaN for in-bounds case, but bounds should be positive
	CHECK(in_bounds[0] > 0);

	// Test with input nearly aligned with control point but outside cone
	Vector3 nearly_aligned_input = Vector3(0.1, 0, 1).normalized();
	in_bounds.write[0] = 0;

	result = cone->closest_to_cone(nearly_aligned_input, &in_bounds);

	// Should handle this gracefully without NaN
	if (in_bounds[0] < 0) { // If out of bounds
		check_vector_finite(result, "nearly aligned input closest point");
	}
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Orthogonal Vector Generation") {
	// Test the get_orthogonal function with various edge cases

	// Test with zero vector
	Vector3 zero_vec = Vector3(0, 0, 0);
	Vector3 ortho_zero = IKLimitCone3D::get_orthogonal(zero_vec);
	check_vector_finite(ortho_zero, "orthogonal to zero vector");

	// Test with unit vectors along each axis
	Vector3 x_axis = Vector3(1, 0, 0);
	Vector3 ortho_x = IKLimitCone3D::get_orthogonal(x_axis);
	check_vector_finite(ortho_x, "orthogonal to X axis");
	CHECK_MESSAGE(Math::is_zero_approx(ortho_x.dot(x_axis)), "Orthogonal vector should be perpendicular to X axis");

	Vector3 y_axis = Vector3(0, 1, 0);
	Vector3 ortho_y = IKLimitCone3D::get_orthogonal(y_axis);
	check_vector_finite(ortho_y, "orthogonal to Y axis");
	CHECK_MESSAGE(Math::is_zero_approx(ortho_y.dot(y_axis)), "Orthogonal vector should be perpendicular to Y axis");

	Vector3 z_axis = Vector3(0, 0, 1);
	Vector3 ortho_z = IKLimitCone3D::get_orthogonal(z_axis);
	check_vector_finite(ortho_z, "orthogonal to Z axis");
	CHECK_MESSAGE(Math::is_zero_approx(ortho_z.dot(z_axis)), "Orthogonal vector should be perpendicular to Z axis");

	// Test with very small vector
	Vector3 tiny_vec = Vector3(1e-10, 1e-10, 1e-10);
	Vector3 ortho_tiny = IKLimitCone3D::get_orthogonal(tiny_vec);
	check_vector_finite(ortho_tiny, "orthogonal to tiny vector");
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Great Tangent Triangle With Parallel Vectors") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create two cones with parallel control points
	Vector3 control_point_a = Vector3(0, 0, 1);
	Vector3 control_point_b = Vector3(0, 0, 1); // Parallel

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, control_point_a, Math::PI / 6);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, control_point_b, Math::PI / 4);

	// Set up tangent handles
	cone_a->update_tangent_handles(cone_b);

	// Test great tangent triangle calculation with various inputs
	Vector3 test_input = Vector3(1, 0, 0);
	Vector3 result = cone_a->get_on_great_tangent_triangle(cone_b, test_input);

	// Should either return a valid point or NaN (indicating not applicable)
	if (!Math::is_nan(result.x)) {
		check_vector_finite(result, "great tangent triangle with parallel vectors");
	}
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Path Sequence With Degenerate Configuration") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create cones in a degenerate configuration
	Vector3 control_point_a = Vector3(1, 0, 0);
	Vector3 control_point_b = Vector3(-1, 0, 0); // Opposite directions

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, control_point_a, Math::PI / 8);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, control_point_b, Math::PI / 8);

	// Set up tangent handles
	cone_a->update_tangent_handles(cone_b);

	// Test path sequence calculation
	Vector3 test_input = Vector3(0, 1, 0);
	Vector3 result = cone_a->get_closest_path_point(cone_b, test_input);

	check_vector_finite(result, "path sequence with degenerate configuration");
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Extreme Radius Values") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Test with very small radius (near zero)
	Vector3 control_point = Vector3(0, 0, 1);
	Ref<IKLimitCone3D> tiny_cone = create_test_cone(kusudama, control_point, 1e-10);

	Vector<double> in_bounds;
	in_bounds.resize(1);
	in_bounds.write[0] = 0;

	Vector3 test_input = Vector3(1, 0, 0);
	Vector3 result = tiny_cone->closest_to_cone(test_input, &in_bounds);

	if (in_bounds[0] < 0) { // If out of bounds
		check_vector_finite(result, "tiny radius cone closest point");
	}

	// Test with very large radius (near π)
	Ref<IKLimitCone3D> large_cone = create_test_cone(kusudama, control_point, Math::PI - 1e-6);

	in_bounds.write[0] = 0;
	result = large_cone->closest_to_cone(test_input, &in_bounds);

	if (in_bounds[0] < 0) { // If out of bounds
		check_vector_finite(result, "large radius cone closest point");
	}
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Mixed Scale Control Points") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	// Create cones with very different magnitude control points (before normalization)
	Vector3 tiny_control = Vector3(1e-10, 0, 1e-10);
	Vector3 huge_control = Vector3(1e10, 0, 1e10);

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, tiny_control, Math::PI / 6);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, huge_control, Math::PI / 4);

	// Both should be normalized properly
	Vector3 normalized_a = cone_a->get_control_point();
	Vector3 normalized_b = cone_b->get_control_point();

	check_vector_valid(normalized_a, "tiny control point normalization");
	check_vector_valid(normalized_b, "huge control point normalization");

	// Test tangent handle calculation
	cone_a->update_tangent_handles(cone_b);

	Vector3 tangent_center_1 = cone_a->get_tangent_circle_center_next_1();
	Vector3 tangent_center_2 = cone_a->get_tangent_circle_center_next_2();

	check_vector_finite(tangent_center_1, "mixed scale tangent center 1");
	check_vector_finite(tangent_center_2, "mixed scale tangent center 2");
}

TEST_CASE("[Modules][ManyBoneIK][IKLimitCone3D] Singularity - Numerical Stability Over Multiple Updates") {
	Ref<IKKusudama3D> kusudama = create_test_kusudama();

	Vector3 control_point_a = Vector3(0, 0, 1);
	Vector3 control_point_b = Vector3(1e-6, 0, 1).normalized(); // Nearly parallel

	Ref<IKLimitCone3D> cone_a = create_test_cone(kusudama, control_point_a, Math::PI / 6);
	Ref<IKLimitCone3D> cone_b = create_test_cone(kusudama, control_point_b, Math::PI / 4);

	// Perform multiple updates to test numerical stability
	for (int i = 0; i < 100; i++) {
		cone_a->update_tangent_handles(cone_b);

		Vector3 tangent_center_1 = cone_a->get_tangent_circle_center_next_1();
		Vector3 tangent_center_2 = cone_a->get_tangent_circle_center_next_2();
		double tangent_radius = cone_a->get_tangent_circle_radius_next();

		// Verify stability over iterations
		check_vector_finite(tangent_center_1, vformat("iteration %d tangent center 1", i));
		check_vector_finite(tangent_center_2, vformat("iteration %d tangent center 2", i));
		CHECK_MESSAGE(Math::is_finite(tangent_radius), vformat("Tangent radius should be finite at iteration %d", i));
	}
}

} // namespace TestIKOpenCone3DSingularities
