/*************************************************************************/
/*  test_dmik.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef TEST_EWBIK_H
#define TEST_EWBIK_H

#include "core/math/basis.h"
#include "../math/qcp.h"

#include "tests/test_macros.h"

namespace TestEWBIK {

Vector3 deg2rad(const Vector3 &p_rotation) {
	return p_rotation / 180.0 * Math_PI;
}

Vector3 rad2deg(const Vector3 &p_rotation) {
	return p_rotation / Math_PI * 180.0;
}

TEST_CASE("[Modules][EWBIK] qcp") {
	QCP *qcp = memnew(QCP);
	qcp->set_max_iterations(10);

	Vector<Vector3> localizedTipHeadings;
	localizedTipHeadings.push_back(Vector3(-14.739, -18.673, 15.040));
	localizedTipHeadings.push_back(Vector3(-12.473, -15.810, 16.074));
	localizedTipHeadings.push_back(Vector3(-14.802, -13.307, 14.408));
	localizedTipHeadings.push_back(Vector3(-17.782, -14.852, 16.171));
	localizedTipHeadings.push_back(Vector3(-16.124, -14.617, 19.584));
	localizedTipHeadings.push_back(Vector3(-15.029, -11.037, 18.902));
	localizedTipHeadings.push_back(Vector3(-18.577, -10.001, 17.996));

	Basis basis;
	Vector3 vec3 = Vector3(0.7, .7, 0.0);
	vec3.normalize();
	basis = basis.rotated(vec3, 0.4);

	Vector<Vector3> localizedTargetHeadings;
	localizedTargetHeadings.resize(7);
	for (int32_t i = 0; i < localizedTargetHeadings.size(); i++) {
		localizedTargetHeadings.write[i] = basis.xform(localizedTipHeadings[i]);
	}

	Vector<float> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	Quat rot;
	Vector3 translation;
	qcp->calc_optimal_rotation(localizedTipHeadings, localizedTargetHeadings,
			weights, rot);
	Quat rot_compare = basis;
	CHECK_MESSAGE(rot.is_equal_approx(rot_compare), vformat("%s does not match quaternion compared %s.", String(rot), String(rot_compare)).utf8().ptr());
	Vector3 euler = Basis(rot).get_euler();
	Vector3 euler_compare = Basis(rot_compare).get_euler();
	CHECK_MESSAGE(euler.is_equal_approx(euler_compare), vformat("%s does not match euler compared %s.", String(euler), String(euler_compare)).utf8().ptr());

	Vector3 axis;
	float angle = 0.0f;
	rot.get_axis_angle(axis, angle);

	Vector<float> checkHeadings;
	checkHeadings.resize(7);
	for (int32_t i = 0; i < checkHeadings.size(); i++) {
		checkHeadings.write[i] = rot.xform(localizedTipHeadings[i]).distance_to(localizedTargetHeadings[i]);
	}
	// TODO Generate arbitrary tests
}
} // namespace TestEWBIK

#endif
