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

#ifndef TEST_QCP_H
#define TEST_QCP_H

#include "core/math/quaternion.h"
#include "modules/many_bone_ik/src/math/qcp.h"
#include "tests/test_macros.h"

namespace TestQCP {

TEST_CASE("[Modules][QCP] No Translation") {
	Quaternion expected = Quaternion(0, 0, sqrt(2) / 2, sqrt(2) / 2);
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	for (Vector3 &element : target) {
		element = expected.xform(element);
	}
	Vector<double> weight = { 1.0, 1.0, 1.0 }; // Equal weights
	bool translate = false;
	double epsilon = 1e-6;
	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved, target, weight, translate, epsilon);
	Quaternion rotation = result[0];
	CHECK(abs(rotation.x - expected.x) < epsilon);
	CHECK(abs(rotation.y - expected.y) < epsilon);
	CHECK(abs(rotation.z - expected.z) < epsilon);
	CHECK(abs(rotation.w - expected.w) < epsilon);
	Vector3 result_translation = result[1];
	CHECK(result_translation.is_zero_approx());
}

TEST_CASE("[Modules][QCP] Different Weights") {
	Quaternion expected = Quaternion(0, 0, sqrt(2) / 2, sqrt(2) / 2);
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	for (Vector3 &element : target) {
		element = expected.xform(element);
	}
	Vector<double> weight = { 0.5, 1.0, 1.5 }; // Different weights
	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved, target, weight, translate, epsilon);
	Quaternion rotation = result[0];
	CHECK(abs(rotation.x - expected.x) < epsilon);
	CHECK(abs(rotation.y - expected.y) < epsilon);
	CHECK(abs(rotation.z - expected.z) < epsilon);
	CHECK(abs(rotation.w - expected.w) < epsilon);
}

TEST_CASE("[Modules][QCP] Zero Weights") {
	Quaternion expected = Quaternion(0, 0, sqrt(2) / 2, sqrt(2) / 2);
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	for (Vector3 &element : target) {
		element = expected.xform(element);
	}
	Vector<double> weight = { 0.0, 0.0, 0.0 }; // Zero weights
	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved, target, weight, translate, epsilon);
	Quaternion rotation = result[0];
	CHECK(abs(rotation.x - expected.x) < epsilon);
	CHECK(abs(rotation.y - expected.y) < epsilon);
	CHECK(abs(rotation.z - expected.z) < epsilon);
	CHECK(abs(rotation.w - expected.w) < epsilon);
}

TEST_CASE("[Modules][QCP] Identity Rotation") {
	Quaternion expected = Quaternion();
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	Vector<double> weight = { 1.0, 1.0, 1.0 }; // Equal weights
	bool translate = false;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved, target, weight, translate, epsilon);
	Quaternion rotation = result[0];
	CHECK(abs(rotation.x - expected.x) < epsilon);
	CHECK(abs(rotation.y - expected.y) < epsilon);
	CHECK(abs(rotation.z - expected.z) < epsilon);
	CHECK(abs(rotation.w - expected.w) < epsilon);
}

TEST_CASE("[Modules][QCP] Random Rotation and Translation") {
	Quaternion expected_rotation = Quaternion(0.1, 0.2, 0.3, 0.4).normalized();
	Vector3 expected_translation = Vector3(1, 2, 3);
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	for (Vector3 &element : target) {
		element = expected_rotation.xform(element + expected_translation);
	}
	Vector<double> weight = { 1.0, 1.0, 1.0 }; // Equal weights
	bool translate = true;
	double epsilon = 1e-6;

	Array result = QuaternionCharacteristicPolynomial::weighted_superpose(moved, target, weight, translate, epsilon);
	Quaternion rotation = result[0];
	Vector3 translation = result[1];
	CHECK(abs(rotation.x - expected_rotation.x) < epsilon);
	CHECK(abs(rotation.y - expected_rotation.y) < epsilon);
	CHECK(abs(rotation.z - expected_rotation.z) < epsilon);
	CHECK(abs(rotation.w - expected_rotation.w) < epsilon);

	CHECK(translate);
	Vector3 translation_result = expected_rotation.xform_inv(translation);
	CHECK(abs(translation_result.x - expected_translation.x) < epsilon);
	CHECK(abs(translation_result.y - expected_translation.y) < epsilon);
	CHECK(abs(translation_result.z - expected_translation.z) < epsilon);
}

} // namespace TestQCP

#endif // TEST_QCP_H
