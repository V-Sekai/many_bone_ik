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
#include "modules/ewbik/math/qcp.h"

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
	localizedTipHeadings.push_back(Vector3(0.0, 0.0, 0.0));
	localizedTipHeadings.push_back(Vector3(0.5219288, 0.1455288, 0.8404827));
	localizedTipHeadings.push_back(Vector3(-0.5219288, -0.1455288, -0.8404827));
	localizedTipHeadings.push_back(Vector3(0.0012719706, -0.9854698, 0.16984463));
	localizedTipHeadings.push_back(Vector3(-0.0012719706, 0.9854698, -0.16984463));
	localizedTipHeadings.push_back(Vector3(0.8529882, -0.08757782, -0.5145302));
	localizedTipHeadings.push_back(Vector3(-0.8529882, 0.08757782, 0.5145302));

	Vector<Vector3> localizedTargetHeadings;
	localizedTargetHeadings.push_back(Vector3(0.014951646, -0.2548256, -0.0037765503));
	localizedTargetHeadings.push_back(Vector3(0.66200894, -0.12242699, 0.7470808));
	localizedTargetHeadings.push_back(Vector3(-0.63210565, -0.3872242, -0.7546339));
	localizedTargetHeadings.push_back(Vector3(0.01495599, -1.2396336, 0.16987133));
	localizedTargetHeadings.push_back(Vector3(0.014947303, 0.7299824, -0.17742443));
	localizedTargetHeadings.push_back(Vector3(0.777393, -0.36718178, -0.64100456));
	localizedTargetHeadings.push_back(Vector3(-0.7474897, -0.1424694, 0.63345146));

	Vector<float> weights;
	weights.push_back(5.0);
	weights.push_back(25.0);
	weights.push_back(25.0);
	weights.push_back(25.0);
	weights.push_back(25.0);
	weights.push_back(25.0);
	weights.push_back(25.0);
	Quat rot;
	Vector3 translation;
	qcp->calc_optimal_rotation(localizedTipHeadings, localizedTargetHeadings,
			weights, rot, false, translation);
	Quat rot_compare;
	rot_compare.x = 0.0019268756;
	rot_compare.y = -0.07416658;
	rot_compare.z = 0.013555937;
	rot_compare.w = -0.99715185;
	CHECK_MESSAGE(rot.is_equal_approx(rot_compare), vformat("%s does not match quaternion compared %s.", String(rot), String(rot_compare)).utf8().ptr());
	Vector3 euler = Basis(rot).get_euler();
	Vector3 euler_compare = Basis(rot_compare).get_euler();
	CHECK_MESSAGE(euler.is_equal_approx(euler_compare), vformat("%s does not match euler compared %s.", String(euler), String(euler_compare)).utf8().ptr());
}
} // namespace TestEWBIK

#endif
