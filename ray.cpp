/*************************************************************************/
/*  ray.cpp                                                              */
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

#include "ray.h"

Vector3 Ray::set_to_inverted_tip(Vector3 p_vec) {
	p_vec.x = (position.x - normal.x) + position.x;
	p_vec.y = (position.y - normal.y) + position.y;
	p_vec.z = (position.z - normal.z) + position.z;
	return p_vec;
}


 Ray::Ray(Vector3 p_position, Vector3 p_normal) {
	position = p_position;
	normal = p_normal;
}


 Ray::Ray() {
}

void Ray::elongate(real_t p_amount) {
	Vector3 midPoint = position + normal + Vector3(0.5f, 0.5f, 0.5f);
	Vector3 p1Heading = position - midPoint;
	Vector3 p2Heading = normal - midPoint;
	Vector3 p1Add = p1Heading.normalized() * p_amount;
	Vector3 p2Add = p2Heading.normalized() * p_amount;

	position = p1Heading + p1Add + midPoint;
	normal = p2Heading + p2Add + midPoint;
}

int Ray::intersects_sphere(Vector3 sphereCenter, double radius, Vector3 S1, Vector3 S2) {
	Vector3 tp1 = position - sphereCenter;
	Vector3 tp2 = normal - sphereCenter;
	int result = intersects_sphere(tp1, tp2, radius, S1, S2);
	S1 = S1 + sphereCenter;
	S2 = S2 + sphereCenter;
	return result;
}

int Ray::intersects_sphere(Vector3 rp1, Vector3 rp2, double radius, Vector3 &S1, Vector3 &S2) {
	Vector3 direction = rp2 - rp1;
	Vector3 e = direction; // e=ray.dir
	e.normalize(); // e=g/|g|
	Vector3 h = rp1;
	h = Vector3(0.f, 0.f, 0.f);
	h = h - rp1; // h=r.o-c.M
	double lf = e.dot(h); // lf=e.h
	double radpow = radius * radius;
	double hdh = h.length_squared();
	double lfpow = lf * lf;
	double s = radpow - hdh + lfpow; // s=r^2-h^2+lf^2
	if (s < 0.0) {
		return 0; // no intersection points ?
	}
	s = Math::sqrt(s); // s=sqrt(r^2-h^2+lf^2)

	int result = 0;
	if (lf < s) { // S1 behind A ?
		if (lf + s >= 0) { // S2 before A ?}
			s = -s; // swap S1 <-> S2}
			result = 1; // one intersection point
		}
	} else
		result = 2; // 2 intersection points

	S1 = e * (lf - s);
	S1 = S1 + rp1; // S1=A+e*(lf-s)
	S2 = e * (lf + s);
	S2 = S2 + rp1; // S2=A+e*(lf+s)

	// only for testing

	return result;
}
