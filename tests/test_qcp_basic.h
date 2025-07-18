/**************************************************************************/
/*  test_qcp_basic.h                                                      */
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

namespace TestQCPBasic {

TEST_CASE("[Modules][QCP] Identity Transformation (No Rotation, No Translation)") {
	PackedVector3Array moved_points = create_simple_point_set();
	PackedVector3Array target_points = moved_points; // Target is identical to moved

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	validate_identity_result(rotation, translation);
}

TEST_CASE("[Modules][QCP] Simple 90-degree Rotation around Z (No Translation)") {
	PackedVector3Array moved_points = create_extended_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);

	Array result = compute_qcp_transformation(moved_points, target_points, false);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	rotation.normalize();
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	CHECK_TRANSLATION_ZERO(translation);
}

TEST_CASE("[Modules][QCP] Simple Translation (No Rotation)") {
	PackedVector3Array moved_points = create_simple_point_set();
	Vector3 expected_translation = Vector3(10, 20, 30);
	PackedVector3Array target_points = apply_translation_only(moved_points, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	rotation.normalize();
	CHECK_IDENTITY_ROTATION(rotation, 1e-6);
	validate_transformation_accuracy(rotation, translation, moved_points, target_points);
}

TEST_CASE("[Modules][QCP] Simple 90-degree Rotation around Z AND Translation") {
	PackedVector3Array moved_points = create_extended_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 2.0);
	Vector3 expected_translation = Vector3(5, -5, 10);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	rotation.normalize();
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	validate_transformation_accuracy(rotation, translation, moved_points, target_points);
}

TEST_CASE("[Modules][QCP] 45-degree Rotation around Y AND Translation") {
	PackedVector3Array moved_points;
	moved_points.push_back(Vector3(1, 0, 0));
	moved_points.push_back(Vector3(0, 1, 0));
	moved_points.push_back(Vector3(0, 0, 1));
	moved_points.push_back(Vector3(2, -1, 3));

	Quaternion expected_rotation = Quaternion(Vector3(0, 1, 0), Math::PI / 4.0);
	Vector3 expected_translation = Vector3(-2, 3, -4);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);

	Array result = compute_qcp_transformation(moved_points, target_points, true);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];

	rotation.normalize();
	CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, 1e-6);
	validate_transformation_accuracy(rotation, translation, moved_points, target_points);
}

TEST_CASE("[Modules][QCP] Multiple Standard Rotations") {
	PackedVector3Array moved_points = create_basic_point_set();
	
	for (int i = 0; i < 5; ++i) {
		const RotationFixture &fixture = STANDARD_ROTATIONS[i];
		PackedVector3Array target_points = apply_transformation(moved_points, fixture.rotation);
		
		Array result = compute_qcp_transformation(moved_points, target_points, false);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];
		
		rotation.normalize();
		CHECK_ROTATION_EQUIVALENT(rotation, fixture.rotation, 1e-6);
		CHECK_TRANSLATION_ZERO(translation);
		validate_geometric_properties(rotation, moved_points, target_points, translation);
	}
}

TEST_CASE("[Modules][QCP] Multiple Standard Translations") {
	PackedVector3Array moved_points = create_simple_point_set();
	
	for (int i = 0; i < 5; ++i) {
		const Vector3 &expected_translation = STANDARD_TRANSLATIONS[i];
		PackedVector3Array target_points = apply_translation_only(moved_points, expected_translation);
		
		Array result = compute_qcp_transformation(moved_points, target_points, true);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];
		
		rotation.normalize();
		CHECK_IDENTITY_ROTATION(rotation, 1e-6);
		validate_transformation_accuracy(rotation, translation, moved_points, target_points);
	}
}

TEST_CASE("[Modules][QCP] Combined Rotation and Translation Matrix") {
	PackedVector3Array moved_points = create_extended_point_set();
	
	for (int rot_idx = 1; rot_idx < 4; ++rot_idx) { // Skip identity rotation
		for (int trans_idx = 1; trans_idx < 4; ++trans_idx) { // Skip zero translation
			const RotationFixture &rot_fixture = STANDARD_ROTATIONS[rot_idx];
			const Vector3 &expected_translation = STANDARD_TRANSLATIONS[trans_idx];
			
			PackedVector3Array target_points = apply_transformation(moved_points, rot_fixture.rotation, expected_translation);
			
			Array result = compute_qcp_transformation(moved_points, target_points, true);
			Quaternion rotation = result[0];
			Vector3 translation = result[1];
			
			rotation.normalize();
			CHECK_ROTATION_EQUIVALENT(rotation, rot_fixture.rotation, 1e-6);
			validate_transformation_accuracy(rotation, translation, moved_points, target_points);
		}
	}
}

TEST_CASE("[Modules][QCP] Precision Validation with Different Tolerances") {
	PackedVector3Array moved_points = create_basic_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(0, 0, 1), Math::PI / 3.0);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation);
	
	for (int i = 0; i < 3; ++i) {
		const ToleranceConfig &config = TOLERANCE_CONFIGS[i];
		
		Array result = compute_qcp_transformation(moved_points, target_points, false, config.standard);
		Quaternion rotation = result[0];
		Vector3 translation = result[1];
		
		rotation.normalize();
		CHECK_ROTATION_EQUIVALENT(rotation, expected_rotation, config.loose);
		CHECK_TRANSLATION_ZERO(translation);
		validate_geometric_properties(rotation, moved_points, target_points, translation);
	}
}

TEST_CASE("[Modules][QCP] Consistency Across Multiple Runs") {
	PackedVector3Array moved_points = create_extended_point_set();
	Quaternion expected_rotation = Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 4.0);
	Vector3 expected_translation = Vector3(1, 2, 3);
	PackedVector3Array target_points = apply_transformation(moved_points, expected_rotation, expected_translation);
	
	Vector<double> weights = create_uniform_weights(moved_points.size());
	validate_canonical_form_consistency(moved_points, target_points, weights, true);
}

} // namespace TestQCPBasic
