/*************************************************************************/
/*  test_ewbik.h                                                         */
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
#include "core/math/vector3.h"
#include "ewbik/math/qcp.h"

#include "tests/test_macros.h"

namespace TestEWBIK {
Vector3 deg2rad(const Vector3 &p_rotation) {
	return p_rotation / 180.0 * Math_PI;
}

Vector3 rad2deg(const Vector3 &p_rotation) {
	return p_rotation / Math_PI * 180.0;
}

void rotate_target_headings_quaternion(Vector<Vector3> &p_localizedTipHeadings, Vector<Vector3> &r_localizedTargetHeadings,
		Quaternion p_rot) {
	for (int32_t i = 0; i < r_localizedTargetHeadings.size(); i++) {
		r_localizedTargetHeadings.write[i] = p_rot.xform(p_localizedTipHeadings[i]);
	}
	Vector<real_t> weights;
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	weights.push_back(1.0);
	Quaternion rot;
	Vector3 translation;
	QCP *qcp = memnew(QCP(1E-6, 1E-11));
	// Axis angle has an opposite theta
	// q0 is the scalar of quaternion
	// QCP uses the other quaternion convention!
	rot = qcp->weighted_superpose(p_localizedTipHeadings, r_localizedTargetHeadings,
			weights, true);
	CHECK_MESSAGE(qcp->get_translation().is_equal_approx(Vector3()), vformat("%s is not zero.", qcp->get_translation()).utf8().ptr());
	memdelete(qcp);
	for (int32_t i = 0; i < p_localizedTipHeadings.size(); i++) {
		Vector3 result = rot.xform(p_localizedTipHeadings[i]);
		float d = result.distance_to(r_localizedTargetHeadings[i]);
		CHECK_MESSAGE(Math::is_zero_approx(d), vformat("%s is not approximately zero. Magnitude difference %s.", d, result.length() / r_localizedTargetHeadings[i].length()).utf8().ptr());
	}
}

TEST_CASE("[Modules][SkeletonModification3DEWBIK] qcp quaternion") {
	Vector<Vector3> localizedTipHeadings;
	localizedTipHeadings.push_back(Vector3(-14.739, -18.673, 15.040));
	localizedTipHeadings.push_back(Vector3(-12.473, -15.810, 16.074));
	localizedTipHeadings.push_back(Vector3(-14.802, -13.307, 14.408));
	localizedTipHeadings.push_back(Vector3(-17.782, -14.852, 16.171));
	localizedTipHeadings.push_back(Vector3(-16.124, -14.617, 19.584));
	localizedTipHeadings.push_back(Vector3(-15.029, -11.037, 18.902));
	localizedTipHeadings.push_back(Vector3(-18.577, -10.001, 17.996));

	Vector<Vector3> localizedTargetHeadings;
	localizedTargetHeadings.resize(7);
	Vector3 norm = Vector3(1.f, 2.0f, 0.0f).normalized();
	Quaternion basis_x = Quaternion(norm, Math_PI / 2.0f);
	rotate_target_headings_quaternion(localizedTipHeadings, localizedTargetHeadings, basis_x);
	Quaternion basis_y = Quaternion(Vector3(0.0f, 1.f, 0.0f), Math_PI / 2.0f);
	rotate_target_headings_quaternion(localizedTipHeadings, localizedTargetHeadings, basis_y);
	Quaternion basis_z = Quaternion(Vector3(0.0f, 0.0f, 1.f), Math_PI / 2.0f);
	rotate_target_headings_quaternion(localizedTipHeadings, localizedTargetHeadings, basis_z);
	basis_x = Quaternion(Vector3(-1.f, 0.0f, 0.0f), Math_PI / 2.0f);
	rotate_target_headings_quaternion(localizedTipHeadings, localizedTargetHeadings, basis_x);
	basis_y = Quaternion(Vector3(0.0f, -1.f, 0.0f), Math_PI / 2.0f);
	rotate_target_headings_quaternion(localizedTipHeadings, localizedTargetHeadings, basis_y);
	basis_z = Quaternion(Vector3(0.0f, 0.0f, -1.f), Math_PI / 2.0f);
	rotate_target_headings_quaternion(localizedTipHeadings, localizedTargetHeadings, basis_z);
}
} // namespace TestEWBIK

#endif
