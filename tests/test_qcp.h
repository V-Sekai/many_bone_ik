#ifndef TEST_QCP_H
#define TEST_QCP_H

#include "core/math/quaternion.h"
#include "modules/many_bone_ik/src/math/qcp.h"
#include "tests/test_macros.h"

namespace TestQCP {

TEST_CASE("[Modules][QCP] Weighted Superpose") {
	double epsilon = CMP_EPSILON;
	QCP qcp(epsilon);

	Quaternion expected = Quaternion(0, 0, sqrt(2) / 2, sqrt(2) / 2);
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	for (Vector3 &element : target) {
		element = expected.xform(element);
	}
	Vector<double> weight = { 1.0, 1.0, 1.0 }; // Equal weights

	Quaternion result = qcp.weighted_superpose(moved, target, weight, false);
	CHECK(abs(result.x - expected.x) < epsilon);
	CHECK(abs(result.y - expected.y) < epsilon);
	CHECK(abs(result.z - expected.z) < epsilon);
	CHECK(abs(result.w - expected.w) < epsilon);
}

TEST_CASE("[Modules][QCP] Weighted Translation") {
	double epsilon = CMP_EPSILON;
	QCP qcp(epsilon);

	Quaternion expected;
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	Vector3 translation_vector = Vector3(1, 2, 3);
	for (Vector3 &element : target) {
		element = expected.xform(element + translation_vector);
	}
	Vector<double> weight = { 1.0, 1.0, 1.0 }; // Equal weights
	bool translate = true;

	Quaternion result = qcp.weighted_superpose(moved, target, weight, translate);
	CHECK(abs(result.x - expected.x) < epsilon);
	CHECK(abs(result.y - expected.y) < epsilon);
	CHECK(abs(result.z - expected.z) < epsilon);
	CHECK(abs(result.w - expected.w) < epsilon);

	// Check if translation occurred
	CHECK(translate == true);
	Vector3 translation_result = expected.xform_inv(qcp.get_translation());
	CHECK(abs(translation_result.x - translation_vector.x) < epsilon);
	CHECK(abs(translation_result.y - translation_vector.y) < epsilon);
	CHECK(abs(translation_result.z - translation_vector.z) < epsilon);
}

TEST_CASE("[Modules][QCP] Weighted Translation Shortest Path") {
	double epsilon = CMP_EPSILON;
	QCP qcp(epsilon);

	Quaternion expected = Quaternion(1, 2, 3, 4).normalized();
	PackedVector3Array moved = { Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3) };
	PackedVector3Array target = moved;
	Vector3 translation_vector = Vector3(1, 2, 3);
	for (Vector3 &element : target) {
		element = expected.xform(element + translation_vector);
	}
	Vector<double> weight = { 1.0, 1.0, 1.0 }; // Equal weights
	bool translate = true;

	Quaternion result = qcp.weighted_superpose(moved, target, weight, translate);
	CHECK(abs(result.x - expected.x) > epsilon);
	CHECK(abs(result.y - expected.y) > epsilon);
	CHECK(abs(result.z - expected.z) > epsilon);
	CHECK(abs(result.w - expected.w) > epsilon);

	// Check if translation occurred
	CHECK(translate == true);
	Vector3 translation_result = expected.xform_inv(qcp.get_translation());
	CHECK(abs(translation_result.x - translation_vector.x) > epsilon);
	CHECK(abs(translation_result.y - translation_vector.y) > epsilon);
	CHECK(abs(translation_result.z - translation_vector.z) > epsilon);
}
} // namespace TestQCP

#endif // TEST_QCP_H
