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

#ifndef TEST_DMIK_H
#define TEST_DMIK_H

#include "../qcp.h"

#include "tests/test_macros.h"

namespace TestDMIK {

	TEST_CASE("[DMIK] DMIK qcp single target") {
	Ref<QCP> qcp;
	qcp.instance();
	qcp->set_max_iterations(10);
	Vector<Vector3> localized_effector_headings;
	localized_effector_headings.push_back(Vector3(0, 0, 0));
	Vector<Vector3> localized_target_headings;
	localized_target_headings.push_back(Vector3(1, 1, 1));
	Quat rot = qcp->weighted_superpose(localized_effector_headings, localized_target_headings,
			Vector<float>(), false);
	Vector3 euler = rot.get_euler();
	INFO(vformat("New orientation in degrees %s x, %s y, %s z", euler.x, euler.y, euler.z).utf8().ptr());
	CHECK_FALSE_MESSAGE(rot.is_equal_approx(Quat()), vformat("%s does not match quat identity", rot).utf8().ptr());
}

TEST_CASE("[DMIK] DMIK qcp two targets") {
	Ref<QCP> qcp;
	qcp.instance();
	qcp->set_max_iterations(10);
	Vector<Vector3> localized_effector_headings;
	localized_effector_headings.push_back(Vector3(0, 0, 0));
	localized_effector_headings.push_back(Vector3(0, 0, 0));
	Vector<Vector3> localized_target_headings;
	localized_target_headings.push_back(Vector3(1, 1, 1));
	localized_target_headings.push_back(Vector3(1, 1, 1));
	Quat rot = qcp->weighted_superpose(localized_effector_headings, localized_target_headings,
			Vector<float>(), false);
	Vector3 euler = rot.get_euler();
	INFO(vformat("New orientation in degrees %s x, %s y, %s z", euler.x, euler.y, euler.z).utf8().ptr());
	CHECK_MESSAGE(rot.is_equal_approx(Quat()), vformat("%s does not match quat identity", rot).utf8().ptr());
}

} // namespace TestDMIK

#endif
