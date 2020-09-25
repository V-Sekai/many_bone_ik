/*************************************************************************/
/*  ik_twist_limit.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "twist_constraint.h"
#include "direction_constraint.h"

void TwistConstraint::set_min_twist_angle(real_t p_min_axial_angle) {
	min_twist_angle = p_min_axial_angle;
}

real_t TwistConstraint::get_min_twist_angle() const {
	return min_twist_angle;
}

void TwistConstraint::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_min_angle_degree", "angle"), &TwistConstraint::set_min_twist_angle_degree);
	ClassDB::bind_method(D_METHOD("get_min_angle_degree"), &TwistConstraint::get_min_twist_angle_degree);
	ClassDB::bind_method(D_METHOD("set_range_degree", "range"), &TwistConstraint::set_range_degree);
	ClassDB::bind_method(D_METHOD("get_range_degree"), &TwistConstraint::get_range_degree);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_angle_degree"), "set_min_angle_degree", "get_min_angle_degree");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "range_degree"), "set_range_degree", "get_range_degree");
}

real_t TwistConstraint::get_range() const {
	return range;
}

void TwistConstraint::set_range(real_t p_range) {
	range = p_range;
}

float TwistConstraint::get_min_twist_angle_degree() const {
	return Math::deg2rad(get_min_twist_angle());
}

void TwistConstraint::set_min_twist_angle_degree(float p_min_axial_angle) {
	set_min_twist_angle(Math::rad2deg(p_min_axial_angle));
}

float TwistConstraint::get_range_degree() const {
	return Math::rad2deg(get_range());
}

void TwistConstraint::set_range_degree(float p_range) {
	set_range(Math::deg2rad(p_range));
}

TwistConstraint::TwistConstraint() {
}

TwistConstraint::~TwistConstraint() {
}
