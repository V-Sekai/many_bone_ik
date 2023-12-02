#include "modules/many_bone_ik/src/ik_kusudama_3d.h"
#include "tests/test_macros.h"

#ifndef TEST_IK_KUDUSAMA_3D_H
#define TEST_IK_KUDUSAMA_3D_H

namespace TestIKKusudama3D {
TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Verify limit cone containment") {
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();

	// Control point for the limit cone (straight up) and 30 degrees cone opening
	Vector3 control_point = Vector3(0, 1, 0);
	real_t radius = Math_PI / 6; // 30 degrees

	// Add a limit cone
	kusudama->add_limit_cone(control_point, radius);

	// Assert that Kusudama correctly sets tangent points based on added cones

	// Get the number of limit cones
	TypedArray<IKLimitCone3D> limit_cones = kusudama->get_limit_cones();
	REQUIRE(limit_cones.size() == 1); // Ensure there is exactly one limit cone

	// Initialize boundary information vector with two elements
	Vector<double> bounds;
	bounds.resize(2);
	bounds.write[0] = 0; // Placeholder for distance from boundary
	bounds.write[1] = 0; // Placeholder for index of limit cone or segment exceeded

	// Test a point outside the bounds – should return a different point from input
	Vector3 test_point_outside = Vector3(0.5, 0.5, 1).normalized();
	Vector3 returned_point_outside = kusudama->get_local_point_in_limits(test_point_outside, &bounds);
	CHECK(returned_point_outside != test_point_outside); // Verify adjustment
	CHECK(bounds[0] < 0); // Negative value indicates outside bounds

	// Test a point inside the bounds – should return the same point as input
	Vector3 test_point_inside = Vector3(0, 0.9, 0.1).normalized();
	Vector3 returned_point_inside = kusudama->get_local_point_in_limits(test_point_inside, &bounds);
	CHECK(returned_point_inside == test_point_inside); // Point remains unchanged
	CHECK(bounds[0] >= 0); // Non-negative value indicates inside bounds or on boundary
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Adding and retrieving Limit Cones") {
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();

	Vector3 point_on_sphere(1, 0, 0); // Unit sphere point
	double radius = Math_PI / 4; // 45 degrees

	kusudama->add_limit_cone(point_on_sphere, radius);

	TypedArray<IKLimitCone3D> limit_cones = kusudama->get_limit_cones();
	CHECK(limit_cones.size() == 1); // Expect one limit cone

	Ref<IKLimitCone3D> retrieved_cone = limit_cones[0];
	CHECK(retrieved_cone.is_valid()); // Validate retrieved cone
	CHECK(Math::is_equal_approx(retrieved_cone->get_radius(), radius)); // Radius check
	CHECK(retrieved_cone->get_closest_path_point(Ref<IKLimitCone3D>(), point_on_sphere) == point_on_sphere);
	CHECK(retrieved_cone->get_closest_path_point(retrieved_cone, point_on_sphere) == point_on_sphere); // Check match

	Vector3 different_point_on_sphere(-1, 0, 0); // Opposite sphere point
	kusudama->add_limit_cone(different_point_on_sphere, radius);

	limit_cones = kusudama->get_limit_cones();
	CHECK(limit_cones.size() == 2); // Now expect two cones

	Ref<IKLimitCone3D> second_retrieved_cone = limit_cones[1];
	CHECK(second_retrieved_cone.is_valid()); // Validate second cone
	CHECK(Math::is_equal_approx(second_retrieved_cone->get_radius(), radius)); // Radius check
	CHECK(second_retrieved_cone->get_closest_path_point(Ref<IKLimitCone3D>(), different_point_on_sphere) == different_point_on_sphere);
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Verify limit cone removal") {
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();

	// Add a couple of limit cones
	Vector3 first_control_point = Vector3(1, 0, 0);
	real_t first_radius = Math_PI / 4; // 45 degrees
	kusudama->add_limit_cone(first_control_point, first_radius);

	Vector3 second_control_point = Vector3(0, 1, 0);
	real_t second_radius = Math_PI / 6; // 30 degrees
	kusudama->add_limit_cone(second_control_point, second_radius);

	// Initial checks (expected two limit cones)
	TypedArray<IKLimitCone3D> limit_cones = kusudama->get_limit_cones();
	REQUIRE(limit_cones.size() == 2);

	// Re-check limit cones
	limit_cones = kusudama->get_limit_cones();

	// Remove the first limit cone
	kusudama->remove_limit_cone(limit_cones[0]);

	// Re-check limit cones
	limit_cones = kusudama->get_limit_cones();
	CHECK(limit_cones.size() == 1); // Only one limit cone should be left
	Ref<IKLimitCone3D> limit_cone = limit_cones[0];
	CHECK(limit_cone->get_control_point() == second_control_point); // Ensure the remaining cone is the correct one
}

TEST_CASE("[Modules][ManyBoneIK][IKKusudama3D] Check limit cones clear functionality") {
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();

	// Add a few limit cones
	kusudama->add_limit_cone(Vector3(1, 0, 0), Math_PI / 4); // 45 degrees
	kusudama->add_limit_cone(Vector3(0, 1, 0), Math_PI / 6); // 30 degrees
	kusudama->add_limit_cone(Vector3(0, 0, 1), Math_PI / 3); // 60 degrees

	// Initial checks (three limit cones expected)
	TypedArray<IKLimitCone3D> limit_cones = kusudama->get_limit_cones();
	REQUIRE(limit_cones.size() == 3);

	// Clear all limit cones
	limit_cones.clear();

	// Re-check limit cones - there should be none
	limit_cones = kusudama->get_limit_cones();
	CHECK(limit_cones.size() == 0); // Expect no limit cones to remain
}
} // namespace TestIKKusudama3D

#endif
