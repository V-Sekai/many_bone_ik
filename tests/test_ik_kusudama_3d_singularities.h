/**************************************************************************/
/*  test_ik_kusudama_3d_singularities.h                                  */
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

namespace TestIKKusudama3DSingularities {

// Helper function to check if a quaternion is valid (finite and normalized)
void check_quaternion_valid(const Quaternion &quat, const String &context = "") {
	CHECK_MESSAGE(quat.is_finite(), vformat("Quaternion should be finite in %s", context));
	CHECK_MESSAGE(!Math::is_nan(quat.x), vformat("Quaternion.x should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(quat.y), vformat("Quaternion.y should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(quat.z), vformat("Quaternion.z should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(quat.w), vformat("Quaternion.w should not be NaN in %s", context));
	
	real_t length_sq = quat.length_squared();
	CHECK_MESSAGE(Math::is_equal_approx(length_sq, real_t(1.0)), vformat("Quaternion should be normalized in %s", context));
}

// Helper function to check if a vector is finite
void check_vector_finite(const Vector3 &vec, const String &context = "") {
	CHECK_MESSAGE(vec.is_finite(), vformat("Vector should be finite in %s", context));
	CHECK_MESSAGE(!Math::is_nan(vec.x), vformat("Vector.x should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(vec.y), vformat("Vector.y should not be NaN in %s", context));
	CHECK_MESSAGE(!Math::is_nan(vec.z), vformat("Vector.z should not be NaN in %s", context));
}

// Helper function to check if a value is finite
void check_value_finite(real_t value, const String &context = "") {
	CHECK_MESSAGE(Math::is_finite(value), vformat("Value should be finite in %s", context));
	CHECK_MESSAGE(!Math::is_nan(value), vformat("Value should not be NaN in %s", context));
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Quaternion From Zero Length Axis") {
	// Test get_quaternion_axis_angle with zero-length axis
	Vector3 zero_axis = Vector3(0, 0, 0);
	real_t angle = Math::PI / 4;
	
	Quaternion result = IKKusudama3D::get_quaternion_axis_angle(zero_axis, angle);
	
	// Should return identity quaternion for zero-length axis
	check_quaternion_valid(result, "zero-length axis quaternion");
	CHECK(result.is_equal_approx(Quaternion()));
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Quaternion From Very Small Axis") {
	// Test get_quaternion_axis_angle with very small axis
	Vector3 tiny_axis = Vector3(1e-10, 1e-10, 1e-10);
	real_t angle = Math::PI / 4;
	
	Quaternion result = IKKusudama3D::get_quaternion_axis_angle(tiny_axis, angle);
	
	// Should handle tiny axis gracefully
	check_quaternion_valid(result, "tiny axis quaternion");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Quaternion From Very Small Angle") {
	// Test get_quaternion_axis_angle with very small angle
	Vector3 axis = Vector3(0, 1, 0);
	real_t tiny_angle = 1e-10;
	
	Quaternion result = IKKusudama3D::get_quaternion_axis_angle(axis, tiny_angle);
	
	// Should return identity quaternion for very small angle
	check_quaternion_valid(result, "tiny angle quaternion");
	CHECK(result.is_equal_approx(Quaternion()));
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Quaternion From Large Values") {
	// Test get_quaternion_axis_angle with large axis values
	Vector3 large_axis = Vector3(1e6, 1e6, 1e6);
	real_t angle = Math::PI / 3;
	
	Quaternion result = IKKusudama3D::get_quaternion_axis_angle(large_axis, angle);
	
	// Should normalize the axis and produce valid quaternion
	check_quaternion_valid(result, "large axis values quaternion");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Swing Twist With Zero Length Axis") {
	// Test get_swing_twist with zero-length axis
	Quaternion rotation = Quaternion(Vector3(1, 0, 0), Math::PI / 4);
	Vector3 zero_axis = Vector3(0, 0, 0);
	Quaternion swing, twist;
	
	IKKusudama3D::get_swing_twist(rotation, zero_axis, swing, twist);
	
	// Should return identity quaternions for zero-length axis
	check_quaternion_valid(swing, "zero-length axis swing");
	check_quaternion_valid(twist, "zero-length axis twist");
	CHECK(swing.is_equal_approx(Quaternion()));
	CHECK(twist.is_equal_approx(Quaternion()));
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Swing Twist With Near Identity Rotation") {
	// Test get_swing_twist with near-identity rotation
	Quaternion near_identity = Quaternion(1e-10, 1e-10, 1e-10, 1.0 - 1e-12).normalized();
	Vector3 axis = Vector3(0, 1, 0);
	Quaternion swing, twist;
	
	IKKusudama3D::get_swing_twist(near_identity, axis, swing, twist);
	
	// Should handle near-identity rotation gracefully
	check_quaternion_valid(swing, "near-identity rotation swing");
	check_quaternion_valid(twist, "near-identity rotation twist");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Swing Twist With 180 Degree Rotation") {
	// Test get_swing_twist with 180-degree rotation (gimbal lock scenario)
	Quaternion rotation_180 = Quaternion(Vector3(1, 0, 0), Math::PI);
	Vector3 axis = Vector3(1, 0, 0); // Same axis as rotation
	Quaternion swing, twist;
	
	IKKusudama3D::get_swing_twist(rotation_180, axis, swing, twist);
	
	// Should handle 180-degree rotation without producing NaN
	check_quaternion_valid(swing, "180-degree rotation swing");
	check_quaternion_valid(twist, "180-degree rotation twist");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Swing Twist With Negative W Quaternion") {
	// Test get_swing_twist with quaternion in negative hemisphere
	Quaternion negative_w = Quaternion(0.1, 0.2, 0.3, -0.9).normalized();
	Vector3 axis = Vector3(0, 1, 0);
	Quaternion swing, twist;
	
	IKKusudama3D::get_swing_twist(negative_w, axis, swing, twist);
	
	// Should handle negative w quaternion correctly
	check_quaternion_valid(swing, "negative w quaternion swing");
	check_quaternion_valid(twist, "negative w quaternion twist");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Swing Twist Decomposition Consistency") {
	// Test that swing * twist = original rotation for various cases
	Vector3 axis = Vector3(0, 1, 0);
	
	// Test with various rotation angles
	real_t test_angles[] = { 0.0, Math::PI / 6, Math::PI / 4, Math::PI / 2, Math::PI, 3 * Math::PI / 2 };
	
	for (int i = 0; i < 6; i++) {
		Quaternion original_rotation = Quaternion(Vector3(1, 1, 1).normalized(), test_angles[i]);
		Quaternion swing, twist;
		
		IKKusudama3D::get_swing_twist(original_rotation, axis, swing, twist);
		
		check_quaternion_valid(swing, vformat("swing at angle %f", test_angles[i]));
		check_quaternion_valid(twist, vformat("twist at angle %f", test_angles[i]));
		
		// Verify swing * twist ≈ original (within tolerance)
		Quaternion reconstructed = swing * twist;
		check_quaternion_valid(reconstructed, vformat("reconstructed at angle %f", test_angles[i]));
		
		// Allow for quaternion double-cover (q and -q represent same rotation)
		bool matches_positive = original_rotation.is_equal_approx(reconstructed);
		bool matches_negative = original_rotation.is_equal_approx(-reconstructed);
		bool matches_either = matches_positive || matches_negative;
		CHECK_MESSAGE(matches_either, vformat("Swing-twist decomposition should be consistent at angle %f", test_angles[i]));
	}
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Degenerate Twist Quaternion") {
	// Test case where twist quaternion becomes degenerate
	Quaternion rotation = Quaternion(0, 0, 0, 1); // Identity
	Vector3 axis = Vector3(0, 1, 0);
	Quaternion swing, twist;
	
	IKKusudama3D::get_swing_twist(rotation, axis, swing, twist);
	
	check_quaternion_valid(swing, "degenerate case swing");
	check_quaternion_valid(twist, "degenerate case twist");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Extreme Floating Point Values") {
	// Test with extreme floating-point values
	Vector3 extreme_axis = Vector3(1e-38, 1e38, 1e-20);
	real_t extreme_angle = 1e-30;
	
	Quaternion result = IKKusudama3D::get_quaternion_axis_angle(extreme_axis, extreme_angle);
	
	// Should handle extreme values gracefully
	check_quaternion_valid(result, "extreme floating-point values");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Clamp To Quadrance Angle Edge Cases") {
	// Test clamp_to_quadrance_angle with edge cases
	
	// Test with identity quaternion
	Quaternion identity = Quaternion();
	double cos_half_angle = Math::cos(Math::PI / 8);
	Quaternion clamped_identity = IKKusudama3D::clamp_to_quadrance_angle(identity, cos_half_angle);
	check_quaternion_valid(clamped_identity, "clamped identity quaternion");
	
	// Test with quaternion at the limit
	Quaternion at_limit = Quaternion(Vector3(1, 0, 0), Math::PI / 4);
	Quaternion clamped_at_limit = IKKusudama3D::clamp_to_quadrance_angle(at_limit, cos_half_angle);
	check_quaternion_valid(clamped_at_limit, "clamped at-limit quaternion");
	
	// Test with quaternion beyond the limit
	Quaternion beyond_limit = Quaternion(Vector3(1, 0, 0), Math::PI / 2);
	Quaternion clamped_beyond = IKKusudama3D::clamp_to_quadrance_angle(beyond_limit, cos_half_angle);
	check_quaternion_valid(clamped_beyond, "clamped beyond-limit quaternion");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Multiple Axis Orientations") {
	// Test swing-twist decomposition with various axis orientations
	Vector3 test_axes[] = {
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(1, 1, 0).normalized(),
		Vector3(1, 0, 1).normalized(),
		Vector3(0, 1, 1).normalized(),
		Vector3(1, 1, 1).normalized()
	};
	
	Quaternion test_rotation = Quaternion(Vector3(0.5, 0.5, 0.7).normalized(), Math::PI / 3);
	
	for (int i = 0; i < 7; i++) {
		Quaternion swing, twist;
		IKKusudama3D::get_swing_twist(test_rotation, test_axes[i], swing, twist);
		
		check_quaternion_valid(swing, vformat("swing for axis %d", i));
		check_quaternion_valid(twist, vformat("twist for axis %d", i));
		
		// Verify twist is actually around the specified axis
		Vector3 twist_axis = twist.get_axis();
		if (!twist.is_equal_approx(Quaternion())) { // Skip for identity quaternion
			real_t axis_alignment = Math::abs(twist_axis.dot(test_axes[i]));
			CHECK_MESSAGE(axis_alignment > 0.9, vformat("Twist should be around specified axis %d", i));
		}
	}
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Numerical Stability Over Iterations") {
	// Test numerical stability over multiple decomposition iterations
	Quaternion rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 6);
	Vector3 axis = Vector3(0, 1, 0);
	
	for (int i = 0; i < 1000; i++) {
		Quaternion swing, twist;
		IKKusudama3D::get_swing_twist(rotation, axis, swing, twist);
		
		check_quaternion_valid(swing, vformat("iteration %d swing", i));
		check_quaternion_valid(twist, vformat("iteration %d twist", i));
		
		// Use the swing as input for next iteration to test stability
		rotation = swing;
		
		// Every 100 iterations, verify we haven't accumulated errors
		if (i % 100 == 99) {
			CHECK_MESSAGE(swing.is_finite(), vformat("Swing should remain finite at iteration %d", i));
			CHECK_MESSAGE(twist.is_finite(), vformat("Twist should remain finite at iteration %d", i));
		}
	}
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Mixed Precision Scenarios") {
	// Test mixed precision scenarios using our new precision functions
	Vector3 axis = Vector3(0, 1, 0);
	Quaternion rotation = Quaternion(Vector3(1, 0, 0), Math::PI / 4);
	
	Quaternion high_prec_swing, high_prec_twist;
	Quaternion low_prec_swing, low_prec_twist;
	
	// Use different precision levels
	IKKusudama3D::get_swing_twist_high_precision(rotation, axis, high_prec_swing, high_prec_twist);
	IKKusudama3D::get_swing_twist_low_precision(rotation, axis, low_prec_swing, low_prec_twist);
	
	check_quaternion_valid(high_prec_swing, "high precision swing");
	check_quaternion_valid(high_prec_twist, "high precision twist");
	check_quaternion_valid(low_prec_swing, "low precision swing");
	check_quaternion_valid(low_prec_twist, "low precision twist");
	
	// High and low precision should produce different results in edge cases
	CHECK(!high_prec_swing.is_equal_approx(low_prec_swing));
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Quaternion Normalization Edge Cases") {
	// Test quaternion normalization in edge cases
	
	// Test with very small quaternion components
	Vector3 tiny_axis = Vector3(1e-20, 1e-20, 1e-20);
	real_t tiny_angle = 1e-15;
	Quaternion tiny_result = IKKusudama3D::get_quaternion_axis_angle(tiny_axis, tiny_angle);
	check_quaternion_valid(tiny_result, "tiny components quaternion");
	
	// Test with very large quaternion components (before normalization)
	Vector3 huge_axis = Vector3(1e20, 1e20, 1e20);
	real_t normal_angle = Math::PI / 4;
	Quaternion huge_result = IKKusudama3D::get_quaternion_axis_angle(huge_axis, normal_angle);
	check_quaternion_valid(huge_result, "huge components quaternion");
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Singularity - Axis Alignment Edge Cases") {
	// Test swing-twist with axis aligned with rotation axis
	Vector3 rotation_axis = Vector3(1, 0, 0);
	Quaternion aligned_rotation = Quaternion(rotation_axis, Math::PI / 3);
	
	Quaternion swing, twist;
	IKKusudama3D::get_swing_twist(aligned_rotation, rotation_axis, swing, twist);
	
	check_quaternion_valid(swing, "axis-aligned swing");
	check_quaternion_valid(twist, "axis-aligned twist");
	
	// For axis-aligned rotation, swing should be identity and twist should be the full rotation
	CHECK(swing.is_equal_approx(Quaternion()));
	bool twist_matches = twist.is_equal_approx(aligned_rotation) || twist.is_equal_approx(-aligned_rotation);
	CHECK(twist_matches);
}

} // namespace TestIKKusudama3DSingularities
