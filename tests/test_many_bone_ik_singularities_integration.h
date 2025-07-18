/**************************************************************************/
/*  test_many_bone_ik_singularities_integration.h                        */
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
#include "modules/many_bone_ik/src/math/ik_node_3d.h"
#include "tests/test_macros.h"

namespace TestManyBoneIKSingularitiesIntegration {

// Helper function to check stability over multiple frames
void check_stability_over_time(const Vector<Quaternion> &rotations, const String &context = "") {
	if (rotations.size() < 2) {
		return;
	}
	
	for (int i = 1; i < rotations.size(); i++) {
		CHECK_MESSAGE(rotations[i].is_finite(), vformat("Rotation should be finite at frame %d in %s", i, context));
		
		// Check for sudden jumps (potential spinning)
		real_t angular_distance = rotations[i].angle_to(rotations[i-1]);
		CHECK_MESSAGE(angular_distance < Math::PI / 2, vformat("Angular change should be reasonable between frames %d and %d in %s", i-1, i, context));
	}
}

// Helper function to create a test constraint setup
Ref<IKKusudama3D> create_upside_down_constraint() {
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();
	
	// Create a constraint that's directly upside down on the forward axis
	Vector3 upside_down_control = Vector3(0, 0, -1); // Pointing backwards (upside down from forward)
	
	Ref<IKLimitCone3D> cone;
	cone.instantiate();
	cone->set_attached_to(kusudama);
	cone->set_control_point(upside_down_control);
	cone->set_radius(Math::PI / 6); // 30 degrees
	
	kusudama->add_open_cone(cone);
	return kusudama;
}

// Helper function to create a test node setup
Ref<IKNode3D> create_test_node(const Vector3 &position = Vector3(), const Quaternion &rotation = Quaternion()) {
	Ref<IKNode3D> node;
	node.instantiate();
	node->set_transform(Transform3D(Basis(rotation), position));
	return node;
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Upside Down Forward Axis Constraint") {
	// This test specifically targets the "upside down on forward axis" singularity
	Ref<IKKusudama3D> kusudama = create_upside_down_constraint();
	
	// Create test nodes
	Ref<IKNode3D> bone_direction = create_test_node();
	Ref<IKNode3D> to_set = create_test_node();
	Ref<IKNode3D> limiting_axes = create_test_node();
	
	// Set up a scenario where the constraint is directly upside down
	Vector3 upside_down_target = Vector3(0, 0, -1); // Directly opposite (upside down)
	
	// Test point that should trigger the singularity
	Vector<double> in_bounds;
	in_bounds.resize(1);
	in_bounds.write[0] = 0;
	
	Vector3 result = kusudama->get_local_point_in_limits(upside_down_target, &in_bounds);
	
	// Should handle the upside-down case without NaN or infinite values
	CHECK(result.is_finite());
	CHECK(!Math::is_nan(result.x));
	CHECK(!Math::is_nan(result.y));
	CHECK(!Math::is_nan(result.z));
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Forward Axis Spinning Prevention") {
	// Test that the constraint doesn't cause spinning when transitioning through the singularity
	Ref<IKKusudama3D> kusudama = create_upside_down_constraint();
	
	Vector<Quaternion> rotations_over_time;
	
	// Simulate movement through the singularity point
	for (int frame = 0; frame < 100; frame++) {
		real_t t = frame / 99.0; // 0 to 1
		
		// Create a path that goes through the upside-down singularity
		Vector3 test_point = Vector3(
			Math::sin(t * Math::PI) * 0.5, // Side movement
			0,
			Math::cos(t * Math::PI) // Forward to backward transition
		).normalized();
		
		Vector<double> in_bounds;
		in_bounds.resize(1);
		in_bounds.write[0] = 0;
		
		Vector3 constrained_point = kusudama->get_local_point_in_limits(test_point, &in_bounds);
		
		// Convert to quaternion for stability analysis
		Quaternion frame_rotation = Quaternion(Vector3(0, 0, 1), constrained_point);
		rotations_over_time.push_back(frame_rotation);
		
		// Verify each frame is stable
		CHECK_MESSAGE(constrained_point.is_finite(), vformat("Constrained point should be finite at frame %d", frame));
	}
	
	// Check overall stability
	check_stability_over_time(rotations_over_time, "forward axis singularity transition");
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Multiple Constraint Interaction") {
	// Test interaction between multiple constraints that could create singularities
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();
	
	// Create multiple cones that could interact problematically
	Vector3 control_points[] = {
		Vector3(0, 0, 1),   // Forward
		Vector3(0, 0, -1),  // Backward (opposite)
		Vector3(1, 0, 0),   // Right
		Vector3(-1, 0, 0)   // Left (opposite)
	};
	
	for (int i = 0; i < 4; i++) {
		Ref<IKLimitCone3D> cone;
		cone.instantiate();
		cone->set_attached_to(kusudama);
		cone->set_control_point(control_points[i]);
		cone->set_radius(Math::PI / 8); // Small radius to increase constraint
		kusudama->add_open_cone(cone);
	}
	
	// Test various points that could trigger singularities
	Vector3 test_points[] = {
		Vector3(0, 1, 0),   // Up (perpendicular to all constraints)
		Vector3(0, -1, 0),  // Down
		Vector3(0.707, 0, 0.707),  // Diagonal
		Vector3(0, 0, 0)    // Origin (degenerate case)
	};
	
	for (int i = 0; i < 4; i++) {
		Vector<double> in_bounds;
		in_bounds.resize(1);
		in_bounds.write[0] = 0;
		
		Vector3 result = kusudama->get_local_point_in_limits(test_points[i], &in_bounds);
		
		CHECK_MESSAGE(result.is_finite(), vformat("Multiple constraint result should be finite for test point %d", i));
		CHECK_MESSAGE(!Math::is_nan(result.x), vformat("Result.x should not be NaN for test point %d", i));
		CHECK_MESSAGE(!Math::is_nan(result.y), vformat("Result.y should not be NaN for test point %d", i));
		CHECK_MESSAGE(!Math::is_nan(result.z), vformat("Result.z should not be NaN for test point %d", i));
	}
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Rapid Orientation Changes") {
	// Test stability under rapid orientation changes that could trigger singularities
	Ref<IKKusudama3D> kusudama = create_upside_down_constraint();
	
	Vector<Quaternion> rapid_rotations;
	
	// Simulate rapid changes in orientation
	for (int i = 0; i < 50; i++) {
		// Create rapidly changing orientations
		real_t angle = i * Math::PI / 4; // 45 degrees per step
		Vector3 axis = Vector3(Math::sin(angle), Math::cos(angle), Math::sin(angle * 2)).normalized();
		Vector3 test_point = Vector3(0, 0, 1).rotated(axis, angle);
		
		Vector<double> in_bounds;
		in_bounds.resize(1);
		in_bounds.write[0] = 0;
		
		Vector3 constrained_point = kusudama->get_local_point_in_limits(test_point, &in_bounds);
		
		CHECK_MESSAGE(constrained_point.is_finite(), vformat("Rapid change result should be finite at step %d", i));
		
		Quaternion step_rotation = Quaternion(Vector3(0, 0, 1), constrained_point);
		rapid_rotations.push_back(step_rotation);
	}
	
	check_stability_over_time(rapid_rotations, "rapid orientation changes");
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Constraint Update Stability") {
	// Test that constraint updates don't introduce instabilities
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();
	Ref<IKNode3D> limiting_axes = create_test_node();
	
	// Create a constraint configuration prone to singularities
	Vector3 nearly_parallel_points[] = {
		Vector3(0, 0, 1),
		Vector3(1e-6, 0, 1).normalized(),
		Vector3(-1e-6, 0, 1).normalized()
	};
	
	for (int i = 0; i < 3; i++) {
		Ref<IKLimitCone3D> cone;
		cone.instantiate();
		cone->set_attached_to(kusudama);
		cone->set_control_point(nearly_parallel_points[i]);
		cone->set_radius(Math::PI / 6);
		kusudama->add_open_cone(cone);
	}
	
	// Perform multiple constraint updates
	for (int update = 0; update < 10; update++) {
		kusudama->_update_constraint(limiting_axes);
		
		// Verify all cones remain stable after update
		TypedArray<IKLimitCone3D> cones = kusudama->get_open_cones();
		for (int i = 0; i < cones.size(); i++) {
			Ref<IKLimitCone3D> cone = cones[i];
			Vector3 control_point = cone->get_control_point();
			
			CHECK_MESSAGE(control_point.is_finite(), vformat("Control point should be finite after update %d, cone %d", update, i));
			CHECK_MESSAGE(!control_point.is_zero_approx(), vformat("Control point should not be zero after update %d, cone %d", update, i));
		}
	}
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Extreme Constraint Configurations") {
	// Test extreme constraint configurations that are prone to singularities
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();
	
	// Configuration 1: Very small radius cones
	Ref<IKLimitCone3D> tiny_cone;
	tiny_cone.instantiate();
	tiny_cone->set_attached_to(kusudama);
	tiny_cone->set_control_point(Vector3(0, 0, 1));
	tiny_cone->set_radius(1e-6); // Extremely small
	kusudama->add_open_cone(tiny_cone);
	
	// Configuration 2: Very large radius cone
	Ref<IKLimitCone3D> huge_cone;
	huge_cone.instantiate();
	huge_cone->set_attached_to(kusudama);
	huge_cone->set_control_point(Vector3(0, 0, -1));
	huge_cone->set_radius(Math::PI - 1e-6); // Nearly full sphere
	kusudama->add_open_cone(huge_cone);
	
	// Test various points with extreme configurations
	Vector3 test_points[] = {
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(0, 0, -1)
	};
	
	for (int i = 0; i < 4; i++) {
		Vector<double> in_bounds;
		in_bounds.resize(1);
		in_bounds.write[0] = 0;
		
		Vector3 result = kusudama->get_local_point_in_limits(test_points[i], &in_bounds);
		
		CHECK_MESSAGE(result.is_finite(), vformat("Extreme configuration result should be finite for test point %d", i));
	}
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Axial Constraint Interaction") {
	// Test interaction between orientational and axial constraints
	Ref<IKKusudama3D> kusudama;
	kusudama.instantiate();
	
	// Set up orientational constraint
	Ref<IKLimitCone3D> cone;
	cone.instantiate();
	cone->set_attached_to(kusudama);
	cone->set_control_point(Vector3(0, 0, -1)); // Upside down
	cone->set_radius(Math::PI / 4);
	kusudama->add_open_cone(cone);
	
	// Set up axial constraint
	kusudama->enable_axial_limits();
	kusudama->set_axial_limits(0, Math::PI); // 180 degree range
	
	// Create test nodes
	Ref<IKNode3D> bone_direction = create_test_node();
	Ref<IKNode3D> to_set = create_test_node();
	Ref<IKNode3D> constraint_axes = create_test_node();
	
	// Test orientation snapping with both constraints active
	kusudama->snap_to_orientation_limit(bone_direction, to_set, constraint_axes, 1.0, 0.9);
	
	// Verify the result is stable
	Transform3D result_transform = to_set->get_transform();
	Quaternion result_rotation = result_transform.basis.get_rotation_quaternion();
	
	CHECK_MESSAGE(result_rotation.is_finite(), "Combined constraint result should be finite");
	CHECK_MESSAGE(result_rotation.is_normalized(), "Combined constraint result should be normalized");
}

TEST_CASE("[Modules][ManyBoneIK][Integration] Singularity - Stress Test Continuous Operation") {
	// Stress test continuous operation through singularity-prone scenarios
	Ref<IKKusudama3D> kusudama = create_upside_down_constraint();
	
	Vector<Vector3> results;
	
	// Run continuous operation for many iterations
	for (int iteration = 0; iteration < 1000; iteration++) {
		// Create varying test points that sweep through potential singularities
		real_t t = iteration / 999.0;
		real_t angle = t * 4 * Math::PI; // Multiple full rotations
		
		Vector3 test_point = Vector3(
			Math::sin(angle) * Math::cos(t * Math::PI),
			Math::sin(angle) * Math::sin(t * Math::PI),
			Math::cos(angle)
		).normalized();
		
		Vector<double> in_bounds;
		in_bounds.resize(1);
		in_bounds.write[0] = 0;
		
		Vector3 result = kusudama->get_local_point_in_limits(test_point, &in_bounds);
		results.push_back(result);
		
		// Verify stability at regular intervals
		if (iteration % 100 == 99) {
			CHECK_MESSAGE(result.is_finite(), vformat("Stress test result should be finite at iteration %d", iteration));
			
			// Check for accumulated drift
			if (results.size() >= 100) {
				Vector3 recent_avg = Vector3();
				for (int j = results.size() - 100; j < results.size(); j++) {
					recent_avg += results[j];
				}
				recent_avg /= 100.0;
				
				CHECK_MESSAGE(recent_avg.is_finite(), vformat("Average result should be finite at iteration %d", iteration));
				CHECK_MESSAGE(recent_avg.length() > 0.1, vformat("Average result should not drift to zero at iteration %d", iteration));
			}
		}
	}
}

} // namespace TestManyBoneIKSingularitiesIntegration
