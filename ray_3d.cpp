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

#include "ray_3d.h"

#include "core/math/vector3.h"

Ray3D::Ray3D() {
}

Ray3D::Ray3D(Vector3 p_origin) {
	this->workingVector = p_origin;
	this->p1_Conflict = p_origin;
}

Ray3D::Ray3D(Vector3 p_p1, Vector3 p_p2) {
	this->workingVector = p_p1;
	this->p1_Conflict = p_p1;
	this->p2_Conflict = p_p2;
}

Vector3 Ray3D::closestPointTo(Vector3 point) {
	workingVector = point;
	workingVector = workingVector - this->p1_Conflict;
	Vector3 heading = this->heading();
	heading.length();
	workingVector.length();
	// workingVector.normalize();
	heading.normalize();
	real_t scale = workingVector.dot(heading);

	return static_cast<Vector3>(this->getScaledTo(scale));
}

Vector3 Ray3D::closestPointToStrict(Vector3 point) {
	Vector3 in_point = point;
	in_point = in_point - this->p1_Conflict;
	Vector3 heading = this->heading();
	real_t scale = (in_point.dot(heading) / (heading.length() * in_point.length())) * (in_point.length() / heading.length());

	if (scale <= 0) {
		return this->p1_Conflict;
	} else if (scale >= 1) {
		return this->p2_Conflict;
	}
	return this->getMultipledBy(scale);
}

Vector3 Ray3D::heading() {
	workingVector = p2_Conflict;
	return workingVector - p1_Conflict;
}

void Ray3D::alignTo(Ref<Ray3D> target) {
	p1_Conflict = target->p1_Conflict;
	p2_Conflict = target->p2_Conflict;
}

void Ray3D::heading(Vector3 newHead) {
	p2_Conflict = p1_Conflict;
	p2_Conflict = newHead;
}

void Ray3D::getHeading(Vector3 setTo) {
	setTo = p2_Conflict;
	setTo -= this->p1_Conflict;
}

Ref<Ray3D> Ray3D::get2DCopy() {
	return this->get2DCopy(Ray3D::Z);
}

Ref<Ray3D> Ray3D::get2DCopy(int collapseOnAxis) {
	Ref<Ray3D> result = this;
	if (collapseOnAxis == Ray3D::X) {
		result->p1_Conflict.x = 0.0f;
		result->p2_Conflict.x = 0.0f;
	}
	if (collapseOnAxis == Ray3D::Y) {
		result->p1_Conflict.y = 0.0f;
		result->p2_Conflict.y = 0.0f;
	}
	if (collapseOnAxis == Ray3D::Z) {
		result->p1_Conflict.z = 0.0f;
		result->p2_Conflict.z = 0.0f;
	}

	return result;
}

Vector3 Ray3D::origin() {
	return p1_Conflict;
}

real_t Ray3D::length() {
	workingVector = p2_Conflict;
	return (workingVector - p1_Conflict).length();
}

void Ray3D::mag(real_t newMag) {
	workingVector = p2_Conflict;
	Vector3 dir = workingVector - p1_Conflict;
	dir = dir * newMag;
	this->heading(dir);
}

real_t Ray3D::scaled_projection(Vector3 input) {
	workingVector = input;
	workingVector = workingVector - this->p1_Conflict;
	Vector3 heading = this->heading();
	real_t headingMag = heading.length();
	real_t workingVectorMag = workingVector.length();
	if (workingVectorMag == 0 || headingMag == 0) {
		return 0;
	}
	return (workingVector.dot(heading) / (headingMag * workingVectorMag)) * (workingVectorMag / headingMag);
}

void Ray3D::div(real_t divisor) {
	p2_Conflict = p2_Conflict - p1_Conflict;
	p2_Conflict = p2_Conflict / divisor;
	p2_Conflict = p2_Conflict + p1_Conflict;
}

void Ray3D::multiply(real_t scalar) {
	p2_Conflict = p2_Conflict - p1_Conflict;
	p2_Conflict = p2_Conflict * scalar;
	p2_Conflict = p2_Conflict + p1_Conflict;
}

Vector3 Ray3D::getMultipledBy(real_t scalar) {
	Vector3 result = this->heading();
	result = result * scalar;
	result = result + p1_Conflict;
	return result;
}

Vector3 Ray3D::getDivideddBy(real_t divisor) {
	Vector3 result = this->heading();
	result = result * divisor;
	result = result + p1_Conflict;
	return result;
}

Vector3 Ray3D::getScaledTo(real_t scale) {
	Vector3 result = this->heading();
	result.normalize();
	result *= scale;
	result += p1_Conflict;
	return result;
}

void Ray3D::elongate(real_t amt) {
	Vector3 midPoint = (p1_Conflict + p2_Conflict) * 0.5f;
	Vector3 p1Heading = p1_Conflict - midPoint;
	Vector3 p2Heading = p2_Conflict - midPoint;
	Vector3 p1Add = p1Heading.normalized() * amt;
	Vector3 p2Add = p2Heading.normalized() * amt;

	this->p1_Conflict = p1Heading + p1Add + midPoint;
	this->p2_Conflict = p2Heading + p2Add + midPoint;
}

Ref<Ray3D> Ray3D::copy() {
	return Ref<Ray3D>(memnew(Ray3D(this->p1_Conflict, this->p2_Conflict)));
}

void Ray3D::reverse() {
	Vector3 temp = this->p1_Conflict;
	this->p1_Conflict = this->p2_Conflict;
	this->p2_Conflict = temp;
}

Ref<Ray3D> Ray3D::getReversed() {
	return memnew(Ray3D(this->p2_Conflict, this->p1_Conflict));
}

Ref<Ray3D> Ray3D::getRayScaledTo(real_t scalar) {
	return memnew(Ray3D(p1_Conflict, this->getScaledTo(scalar)));
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
	return Ref<Ray3D>(memnew(Ray3D(p1_Conflict, this->getMultipledBy(scalar))));
}

Vector3 Ray3D::setToInvertedTip(Vector3 vec) {
	vec.x = (p1_Conflict.x - p2_Conflict.x) + p1_Conflict.x;
	vec.y = (p1_Conflict.y - p2_Conflict.y) + p1_Conflict.y;
	vec.z = (p1_Conflict.z - p2_Conflict.z) + p1_Conflict.z;
	return vec;
}

void Ray3D::contractTo(real_t percent) {
	// contracts both ends of a ray toward its center such that the total length of
	// the ray is the percent % of its current length;
	real_t halfPercent = 1 - ((1 - percent) / 2.0f);

	p1_Conflict = p1_Conflict.lerp(p2_Conflict, halfPercent);
	p2_Conflict = p2_Conflict.lerp(p1_Conflict, halfPercent);
}

void Ray3D::translateTo(Vector3 newLocation) {
	workingVector = p2_Conflict;
	workingVector = workingVector - p1_Conflict;
	workingVector = workingVector + newLocation;
	p2_Conflict = workingVector;
	p1_Conflict = newLocation;
}

void Ray3D::translateTipTo(Vector3 newLocation) {
	workingVector = newLocation;
	Vector3 transBy = workingVector - p2_Conflict;
	this->translateBy(transBy);
}

void Ray3D::translateBy(Vector3 toAdd) {
	p1_Conflict += toAdd;
	p2_Conflict += toAdd;
}

void Ray3D::normalize() {
	this->mag(1);
}

Vector3 Ray3D::intercepts2D(Ref<Ray3D> r) {
	Vector3 result = p1_Conflict;

	real_t p0_x = this->p1_Conflict.x;
	real_t p0_y = this->p1_Conflict.y;
	real_t p1_x = this->p2_Conflict.x;
	real_t p1_y = this->p2_Conflict.y;

	real_t p2_x = r->p1_Conflict.x;
	real_t p2_y = r->p1_Conflict.y;
	real_t p3_x = r->p2_Conflict.x;
	real_t p3_y = r->p2_Conflict.y;

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
	return this->closestPointTo(closestToThis);
}

Vector3 Ray3D::closestPointToRay3D(Ref<Ray3D> r) {
	workingVector = p2_Conflict;
	Vector3 u = workingVector - this->p1_Conflict;
	workingVector = r->p2_Conflict;
	Vector3 v = workingVector - r->p1_Conflict;
	workingVector = this->p1_Conflict;
	Vector3 w = workingVector - r->p1_Conflict;
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
	workingVector = p2_Conflict;
	Vector3 u = workingVector - this->p1_Conflict;
	workingVector = r->p2_Conflict;
	Vector3 v = workingVector - r->p1_Conflict;
	workingVector = this->p1_Conflict;
	Vector3 w = workingVector - r->p1_Conflict;
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
		result = this->p1_Conflict;
	} else if (sc > 1) {
		result = this->p2_Conflict;
	} else {
		result = this->getRayScaledBy(sc)->p2();
	}

	return result;
}

Ref<Ray3D> Ray3D::getPerpendicular2D() {
	Vector3 heading = this->heading();
	workingVector = Vector3(heading.x - 1.0f, heading.x, 0.0f);
	return Ref<Ray3D>(memnew(Ray3D(this->p1_Conflict, workingVector + this->p1_Conflict)));
}

Vector3 Ray3D::intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc) {
	Vector3 uvw;
	tta = ta;
	ttb = tb;
	ttc = tc;
	tta -= p1_Conflict;
	ttb -= p1_Conflict;
	ttc -= p1_Conflict;

	Vector3 result = planeIntersectTest(tta, ttb, ttc, uvw);
	return result + p1_Conflict;
}

int Ray3D::intersectsSphere(Vector3 sphereCenter, real_t radius, Vector3 S1, Vector3 S2) {
	Vector3 tp1 = p1_Conflict - sphereCenter;
	Vector3 tp2 = p2_Conflict - sphereCenter;
	int result = intersectsSphere(tp1, tp2, radius, S1, S2);
	S1 += sphereCenter;
	S2 += sphereCenter;
	return result;
}

void Ray3D::p1(Vector3 in) {
	this->p1_Conflict = in;
}

void Ray3D::p2(Vector3 in) {
	this->p2_Conflict = in;
}

Vector3 Ray3D::p2() {
	return p2_Conflict;
}

void Ray3D::setP2(Vector3 p_p2) {
	this->p2_Conflict = p_p2;
}

Vector3 Ray3D::p1() {
	return p1_Conflict;
}

void Ray3D::setP1(Vector3 p_p1) {
	this->p1_Conflict = p_p1;
}

int Ray3D::intersectsSphere(Vector3 rp1, Vector3 rp2, float radius, Vector3 S1, Vector3 S2) {
	Vector3 direction = static_cast<Vector3>(rp2 - rp1);
	Vector3 e = direction; // e=ray.dir
	e.normalize(); // e=g/|g|
	Vector3 h = p1_Conflict;
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