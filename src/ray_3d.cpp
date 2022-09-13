/*************************************************************************/
/*  ray_3d.cpp                                                           */
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

#include "ray_3d.h"

#include "core/math/vector3.h"

Ray3D::Ray3D() {
}

Ray3D::Ray3D(Vector3 p_p1, Vector3 p_p2) {
	this->workingVector = p_p1;
	this->_p1 = p_p1;
	this->_p2 = p_p2;
}

Vector3 Ray3D::closest_point_to(const Vector3 p_point) {
	workingVector = p_point;
	workingVector = workingVector - this->_p1;
	Vector3 heading = this->heading();
	heading.length();
	workingVector.length();
	// workingVector.normalize();
	heading.normalize();
	real_t scale = workingVector.dot(heading);

	return static_cast<Vector3>(get_scaled_to(scale));
}

Vector3 Ray3D::closest_point_to_strict(const Vector3 &p_point) {
	Vector3 in_point = p_point;
	in_point = in_point - this->_p1;
	Vector3 heading = this->heading();
	real_t scale = (in_point.dot(heading) / (heading.length() * in_point.length())) * (in_point.length() / heading.length());

	if (scale <= 0) {
		return this->_p1;
	} else if (scale >= 1) {
		return this->_p2;
	}
	return this->get_multipled_by(scale);
}

Vector3 Ray3D::heading() {
	workingVector = _p2;
	return workingVector - _p1;
}

void Ray3D::set_align_to(Ref<Ray3D> p_target) {
	_p1 = p_target->_p1;
	_p2 = p_target->_p2;
}

void Ray3D::set_heading(Vector3 &p_new_head) {
	_p2 = _p1;
	_p2 = p_new_head;
}

Vector3 Ray3D::get_heading(const Vector3 &r_set_to) const {
	Vector3 set_to;
	set_to = _p2;
	set_to -= this->_p1;
	return set_to;
}

Ref<Ray3D> Ray3D::get_2d_copy() {
	return get_2d_copy(Ray3D::Z);
}

Ref<Ray3D> Ray3D::get_2d_copy(const int &p_collapse_on_axis) {
	Ref<Ray3D> result = this;
	if (p_collapse_on_axis == Ray3D::X) {
		result->_p1.x = 0.0f;
		result->_p2.x = 0.0f;
	}
	if (p_collapse_on_axis == Ray3D::Y) {
		result->_p1.y = 0.0f;
		result->_p2.y = 0.0f;
	}
	if (p_collapse_on_axis == Ray3D::Z) {
		result->_p1.z = 0.0f;
		result->_p2.z = 0.0f;
	}

	return result;
}

Vector3 Ray3D::get_origin() {
	return _p1;
}

real_t Ray3D::get_length() {
	workingVector = _p2;
	return (workingVector - _p1).length();
}

void Ray3D::set_magnitude(const real_t &p_new_mag) {
	workingVector = _p2;
	Vector3 dir = workingVector - _p1;
	dir = dir * p_new_mag;
	this->set_heading(dir);
}

real_t Ray3D::scaled_projection(const Vector3 &p_input) {
	workingVector = p_input;
	workingVector = workingVector - this->_p1;
	Vector3 heading = this->heading();
	real_t headingMag = heading.length();
	real_t workingVectorMag = workingVector.length();
	if (workingVectorMag == 0 || headingMag == 0) {
		return 0;
	}
	return (workingVector.dot(heading) / (headingMag * workingVectorMag)) * (workingVectorMag / headingMag);
}

void Ray3D::set_divide(const real_t &p_divisor) {
	_p2 = _p2 - _p1;
	_p2 = _p2 / p_divisor;
	_p2 = _p2 + _p1;
}

void Ray3D::set_multiply(const real_t &p_scalar) {
	_p2 = _p2 - _p1;
	_p2 = _p2 * p_scalar;
	_p2 = _p2 + _p1;
}

Vector3 Ray3D::get_multipled_by(const real_t &p_scalar) {
	Vector3 result = this->heading();
	result = result * p_scalar;
	result = result + _p1;
	return result;
}

Vector3 Ray3D::get_divided_by(const real_t &p_divisor) {
	Vector3 result = heading();
	result = result * p_divisor;
	result = result + _p1;
	return result;
}

Vector3 Ray3D::get_scaled_to(const real_t &scale) {
	Vector3 result = heading();
	result.normalize();
	result *= scale;
	result += _p1;
	return result;
}

void Ray3D::elongate(real_t amt) {
	Vector3 midPoint = (_p1 + _p2) * 0.5f;
	Vector3 p1Heading = _p1 - midPoint;
	Vector3 p2Heading = _p2 - midPoint;
	Vector3 p1Add = p1Heading.normalized() * amt;
	Vector3 p2Add = p2Heading.normalized() * amt;

	this->_p1 = p1Heading + p1Add + midPoint;
	this->_p2 = p2Heading + p2Add + midPoint;
}

Ref<Ray3D> Ray3D::copy() {
	return Ref<Ray3D>(memnew(Ray3D(this->_p1, this->_p2)));
}

void Ray3D::reverse() {
	Vector3 temp = this->_p1;
	this->_p1 = this->_p2;
	this->_p2 = temp;
}

Ref<Ray3D> Ray3D::getReversed() {
	return memnew(Ray3D(this->_p2, this->_p1));
}

Ref<Ray3D> Ray3D::get_ray_scaled_to(real_t scalar) {
	return memnew(Ray3D(_p1, get_scaled_to(scalar)));
}

void Ray3D::pointWith(Ref<Ray3D> r) {
	if (this->heading().dot(r->heading()) < 0) {
		this->reverse();
	}
}

void Ray3D::pointWith(Vector3 heading) {
	if (this->heading().dot(heading) < 0) {
		this->reverse();
	}
}

Ref<Ray3D> Ray3D::getRayScaledBy(real_t scalar) {
	return Ref<Ray3D>(memnew(Ray3D(_p1, get_multipled_by(scalar))));
}

Vector3 Ray3D::setToInvertedTip(Vector3 vec) {
	vec.x = (_p1.x - _p2.x) + _p1.x;
	vec.y = (_p1.y - _p2.y) + _p1.y;
	vec.z = (_p1.z - _p2.z) + _p1.z;
	return vec;
}

void Ray3D::contractTo(real_t percent) {
	// contracts both ends of a ray toward its center such that the total length of
	// the ray is the percent % of its current length;
	real_t halfPercent = 1 - ((1 - percent) / 2.0f);

	_p1 = _p1.lerp(_p2, halfPercent);
	_p2 = _p2.lerp(_p1, halfPercent);
}

void Ray3D::translateTo(Vector3 newLocation) {
	workingVector = _p2;
	workingVector = workingVector - _p1;
	workingVector = workingVector + newLocation;
	_p2 = workingVector;
	_p1 = newLocation;
}

void Ray3D::translateTipTo(Vector3 newLocation) {
	workingVector = newLocation;
	Vector3 transBy = workingVector - _p2;
	this->translateBy(transBy);
}

void Ray3D::translateBy(Vector3 toAdd) {
	_p1 += toAdd;
	_p2 += toAdd;
}

void Ray3D::normalize() {
	this->set_magnitude(1);
}

Vector3 Ray3D::intercepts2D(Ref<Ray3D> r) {
	Vector3 result = _p1;

	real_t p0_x = this->_p1.x;
	real_t p0_y = this->_p1.y;
	real_t p1_x = this->_p2.x;
	real_t p1_y = this->_p2.y;

	real_t p2_x = r->_p1.x;
	real_t p2_y = r->_p1.y;
	real_t p3_x = r->_p2.x;
	real_t p3_y = r->_p2.y;

	real_t s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;
	s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;
	s2_y = p3_y - p2_y;

	real_t t;
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	return result = Vector3(p0_x + (t * s1_x), p0_y + (t * s1_y), 0.0f);
}

Vector3 Ray3D::closestPointToSegment3D(Ref<Ray3D> r) {
	Vector3 closestToThis = r->closestPointToRay3DStrict(this);
	return this->closest_point_to(closestToThis);
}

Vector3 Ray3D::closestPointToRay3D(Ref<Ray3D> r) {
	workingVector = _p2;
	Vector3 u = workingVector - this->_p1;
	workingVector = r->_p2;
	Vector3 v = workingVector - r->_p1;
	workingVector = this->_p1;
	Vector3 w = workingVector - r->_p1;
	real_t a = u.dot(u);
	real_t b = u.dot(v);
	real_t c = v.dot(v);
	real_t d = u.dot(w);
	real_t e = v.dot(w);
	real_t D = a * c - b * b;
	real_t sc;

	// compute the line parameters of the two closest points
	if (D < FLT_TRUE_MIN) {
		sc = 0.0f;
	} else {
		sc = (b * e - c * d) / D;
	}

	return getRayScaledBy(sc)->p2();
}

Vector3 Ray3D::closestPointToRay3DStrict(Ref<Ray3D> r) {
	workingVector = _p2;
	Vector3 u = workingVector - this->_p1;
	workingVector = r->_p2;
	Vector3 v = workingVector - r->_p1;
	workingVector = this->_p1;
	Vector3 w = workingVector - r->_p1;
	real_t a = u.dot(u); // always >= s0
	real_t b = u.dot(v);
	real_t c = v.dot(v); // always >= 0
	real_t d = u.dot(w);
	real_t e = v.dot(w);
	real_t D = a * c - b * b; // always >= 0
	real_t sc; // tc

	// compute the line parameters of the two closest points
	if (D < FLT_TRUE_MIN) {
		sc = 0.0f;
	} else {
		sc = (b * e - c * d) / D;
	}

	Vector3 result;
	if (sc < 0) {
		result = this->_p1;
	} else if (sc > 1) {
		result = this->_p2;
	} else {
		result = this->getRayScaledBy(sc)->p2();
	}

	return result;
}

Ref<Ray3D> Ray3D::getPerpendicular2D() {
	Vector3 heading = this->heading();
	workingVector = Vector3(heading.x - 1.0f, heading.x, 0.0f);
	return Ref<Ray3D>(memnew(Ray3D(this->_p1, workingVector + this->_p1)));
}

Vector3 Ray3D::intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc) {
	Vector3 uvw;
	tta = ta;
	ttb = tb;
	ttc = tc;
	tta -= _p1;
	ttb -= _p1;
	ttc -= _p1;

	Vector3 result = planeIntersectTest(tta, ttb, ttc, uvw);
	return result + _p1;
}

int Ray3D::intersectsSphere(Vector3 sphereCenter, real_t radius, Vector3 &S1, Vector3 &S2) {
	Vector3 tp1 = _p1 - sphereCenter;
	Vector3 tp2 = _p2 - sphereCenter;
	int result = intersectsSphere(tp1, tp2, radius, S1, S2);
	S1 += sphereCenter;
	S2 += sphereCenter;
	return result;
}

void Ray3D::p1(Vector3 in) {
	this->_p1 = in;
}

void Ray3D::p2(Vector3 in) {
	this->_p2 = in;
}

Vector3 Ray3D::p2() {
	return _p2;
}

void Ray3D::setP2(Vector3 p_p2) {
	this->_p2 = p_p2;
}

Vector3 Ray3D::p1() {
	return _p1;
}

void Ray3D::setP1(Vector3 p_p1) {
	this->_p1 = p_p1;
}

int Ray3D::intersectsSphere(Vector3 rp1, Vector3 rp2, float radius, Vector3 &S1, Vector3 &S2) {
	Vector3 direction = rp2 - rp1;
	Vector3 e = direction; // e=ray.dir
	e.normalize(); // e=g/|g|
	Vector3 h = _p1;
	h = Vector3(0.0f, 0.0f, 0.0f);
	h = h - rp1; // h=r.o-c.M
	float lf = e.dot(h); // lf=e.h
	float radpow = radius * radius;
	float hdh = h.length_squared();
	float lfpow = lf * lf;
	float s = radpow - hdh + lfpow; // s=r^2-h^2+lf^2
	if (s < 0.0f) {
		return 0; // no intersection points ?
	}
	s = Math::sqrt(s); // s=sqrt(r^2-h^2+lf^2)

	int result = 0;
	if (lf < s) {
		if (lf + s >= 0) {
			s = -s; // swap S1 <-> S2}
			result = 1; // one intersection point
		}
	} else {
		result = 2; // 2 intersection points
	}

	S1 = e * (lf - s);
	S1 += rp1; // S1=A+e*(lf-s)
	S2 = e * (lf + s);
	S2 += rp1; // S2=A+e*(lf+s)
	return result;
}
