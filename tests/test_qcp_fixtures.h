/**************************************************************************/
/*  test_qcp_fixtures.h                                                   */
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

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/math/quaternion.h"
#include "core/math/vector3.h"
#include "core/variant/variant.h"

namespace TestQCPFixtures {

// Standard rotation fixtures
struct RotationFixture {
	Quaternion rotation;
	Vector3 axis;
	double angle;
	const char *description;
};

static const RotationFixture STANDARD_ROTATIONS[] = {
	{ Quaternion(), Vector3(0, 0, 1), 0.0, "Identity" },
	{ Quaternion(Vector3(0, 0, 1), Math::PI / 2.0), Vector3(0, 0, 1), Math::PI / 2.0, "90 degrees around Z" },
	{ Quaternion(Vector3(0, 1, 0), Math::PI / 4.0), Vector3(0, 1, 0), Math::PI / 4.0, "45 degrees around Y" },
	{ Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 6.0), Vector3(1, 1, 1).normalized(), Math::PI / 6.0, "30 degrees around (1,1,1)" },
	{ Quaternion(Vector3(1, 1, 1).normalized(), Math::PI / 3.0), Vector3(1, 1, 1).normalized(), Math::PI / 3.0, "60 degrees around (1,1,1)" }
};

// Standard translation fixtures
static const Vector3 STANDARD_TRANSLATIONS[] = {
	Vector3(0, 0, 0), // No translation
	Vector3(10, 20, 30), // Simple translation
	Vector3(5, -5, 10), // Mixed signs
	Vector3(-2, 3, -4), // All negative/positive mix
	Vector3(1, 2, 3) // Small translation
};

// Edge case point sets
inline PackedVector3Array create_nearly_parallel_vectors() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 0, 0));
	points.push_back(Vector3(1, 1e-10, 0)); // Nearly parallel to first point
	points.push_back(Vector3(0, 0, 1));
	return points;
}

inline PackedVector3Array create_small_value_points() {
	PackedVector3Array points;
	points.push_back(Vector3(1e-8, 0, 0));
	points.push_back(Vector3(0, 1e-8, 0));
	points.push_back(Vector3(0, 0, 1e-8));
	return points;
}

inline PackedVector3Array create_large_value_points() {
	PackedVector3Array points;
	points.push_back(Vector3(1e6, 0, 0));
	points.push_back(Vector3(0, 1e6, 0));
	points.push_back(Vector3(0, 0, 1e6));
	return points;
}

inline PackedVector3Array create_complex_multi_point_set() {
	PackedVector3Array points;
	points.push_back(Vector3(1, 0, 0));
	points.push_back(Vector3(0, 1, 0));
	points.push_back(Vector3(0, 0, 1));
	points.push_back(Vector3(1, 1, 0));
	points.push_back(Vector3(1, 0, 1));
	points.push_back(Vector3(0, 1, 1));
	return points;
}

inline PackedVector3Array create_performance_test_points(int count = 100) {
	PackedVector3Array points;
	for (int i = 0; i < count; ++i) {
		Vector3 point = Vector3(
				Math::sin(i * 0.1) * 10,
				Math::cos(i * 0.1) * 10,
				i * 0.1);
		points.push_back(point);
	}
	return points;
}

// Special weight configurations
inline Vector<double> create_negative_weight_config() {
	Vector<double> weights;
	weights.push_back(-1.0); // Negative weight
	weights.push_back(1.0);
	weights.push_back(1.0);
	return weights;
}

// Tolerance configurations for different test scenarios
struct ToleranceConfig {
	double standard;
	double loose;
	double strict;
	const char *description;
};

static const ToleranceConfig TOLERANCE_CONFIGS[] = {
	{ 1e-6, 1e-3, 1e-10, "Standard precision" },
	{ 1e-5, 0.1, 1e-8, "Edge case tolerance" },
	{ 1e-3, 0.1, 1e-6, "Large value tolerance" }
};

} // namespace TestQCPFixtures
