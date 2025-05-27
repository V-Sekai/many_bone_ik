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
	CHECK( (Math::abs(rotation.x - expected_rotation.x) < epsilon || Math::abs(rotation.x + expected_rotation.x) < epsilon) );
	CHECK( (Math::abs(rotation.y - expected_rotation.y) < epsilon || Math::abs(rotation.y + expected_rotation.y) < epsilon) );
	CHECK( (Math::abs(rotation.z - expected_rotation.z) < epsilon || Math::abs(rotation.z + expected_rotation.z) < epsilon) );
	CHECK( (Math::abs(rotation.w - expected_rotation.w) < epsilon || Math::abs(rotation.w + expected_rotation.w) < epsilon) );
	CHECK( Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon );

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

	CHECK( Math::abs(Math::abs(rotation.dot(expected_rotation)) - 1.0) < epsilon );

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

	CHECK( Math::abs(Math::abs(rotation_result.dot(expected_rotation)) - 1.0) < epsilon );

	Vector3 calculated_translation_component = expected_rotation.xform_inv(translation_qcp);
	CHECK( (calculated_translation_component - expected_translation_component).length() < epsilon );
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

	CHECK( Math::abs(Math::abs(rotation_result.dot(expected_rotation)) - 1.0) < epsilon );
	CHECK( (translation_qcp - expected_translation_component).length() < epsilon );
}

} // namespace TestQCP
