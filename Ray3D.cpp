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

#include "Ray3D.h"

Ray3D::Ray3D() {
	workingVector = new Vector3();
}

Ray3D::Ray3D(Vector3 p_origin) {
	this->workingVector = p_origin;
	this->p1_Conflict = p_origin;
}

Ray3D::Ray3D(Vector3 p_p1, Vector3 p_p2) {
	this->workingVector = p_p1;
	this->p1_Conflict = p_p1;
	if (p2 != nullptr) {
		this->p2_Conflict = p_p2;
	}
}

ewbik::math::Ray3D *Ray3D::planePlaneIntersect(Vector3 p_a1, Vector3 p_a2, Vector3 p_a3, Vector3 p_b1, Vector3 p_b2, Vector3 p_b3) {
	ewbik::math::Ray3D *a1a2 = new ewbik::math::Ray3D(p_a1, p_a2);
	ewbik::math::Ray3D *a1a3 = new ewbik::math::Ray3D(p_a1, p_a3);
	ewbik::math::Ray3D *a2a3 = new ewbik::math::Ray3D(p_a2, p_a3);

	Vector3 interceptsa1a2 = a1a2->intersectsPlane(p_b1, p_b2, p_b3);
	Vector3 interceptsa1a3 = a1a3->intersectsPlane(p_b1, p_b2, p_b3);
	Vector3 interceptsa2a3 = a2a3->intersectsPlane(p_b1, p_b2, p_b3);

	std::vector<Vector3 > notNullCandidates = { interceptsa1a2, interceptsa1a3, interceptsa2a3 };
	Vector3 notNull1 = nullptr;
	Vector3 notNull2 = nullptr;

	for (int i = 0; i < notNullCandidates.size(); i++) {
		if (notNullCandidates[i] != nullptr) {
			if (notNull1 == nullptr) {
				notNull1 = notNullCandidates[i];
			} else {
				notNull2 = notNullCandidates[i];
				break;
			}
		}
	}
	if (notNull1 != nullptr && notNull2 != nullptr) {
		// delete a2a3;
		// delete a1a3;
		// delete a1a2;
		return new ewbik::math::Ray3D(notNull1, notNull2);
	} else {
		// delete a2a3;
		// delete a1a3;
		// delete a1a2;
		return nullptr;
	}

	// delete a2a3;
	// delete a1a3;
	// delete a1a2;
}

float Ray3D::triArea2D(float x1, float y1, float x2, float y2, float x3, float y3) {
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
}

float Ray3D::distTo(Vector3 point) {
	Vector3 inPoint = point->copy();
	inPoint->sub(this->p1_Conflict);
	Vector3 heading = this->heading();
	float scale = (inPoint->dot(heading) / (heading->mag() * inPoint->mag())) * (inPoint->mag() / heading->mag());

	return point->dist(this->getRayScaledBy(scale)->p2);
}

float Ray3D::distToStrict(Vector3 point) {
	Vector3 inPoint = point->copy();
	inPoint->sub(this->p1_Conflict);
	Vector3 heading = this->heading();
	float scale = (inPoint->dot(heading) / (heading->mag() * inPoint->mag())) * (inPoint->mag() / heading->mag());
	if (scale < 0) {
		return point->dist(this->p1_Conflict);
	} else if (scale > 1) {
		return point->dist(this->p2_Conflict);
	} else {
		return point->dist(this->getRayScaledBy(scale)->p2);
	}
}

float Ray3D::distTo(ewbik::math::Ray3D *r) {
	Vector3 closestOnThis = this->closestPointToRay3D(r);
	return r->distTo(closestOnThis);
}

float Ray3D::distToStrict(ewbik::math::Ray3D *r) {
	Vector3 closestOnThis = this->closestPointToSegment3D(r);
	return closestOnThis->dist(r->closestPointToStrict(closestOnThis));
}

Vector3 Ray3D::closestPointTo(Vector3 point) {
	workingVector->set(point);
	workingVector->sub(this->p1_Conflict);
	Vector3 heading = this->heading();
	heading->mag();
	workingVector->mag();
	// workingVector.normalize();
	heading->normalize();
	float scale = workingVector->dot(heading);

	return static_cast<Vector3 >(this->getScaledTo(scale));
}

Vector3 Ray3D::closestPointToStrict(Vector3 point) {
	Vector3 inPoint = static_cast<Vector3 >(point->copy());
	inPoint->sub(this->p1_Conflict);
	Vector3 heading = static_cast<Vector3 >(this->heading());
	float scale = (inPoint->dot(heading) / (heading->mag() * inPoint->mag())) * (inPoint->mag() / heading->mag());

	if (scale <= 0) {
		return this->p1_Conflict;
	} else if (scale >= 1) {
		return this->p2_Conflict;
	} else {
		return this->getMultipledBy(scale);
	}
}

Vector3 Ray3D::heading() {
	if (this->p2_Conflict == nullptr) {
		if (p1_Conflict == nullptr) {
			p1_Conflict = new Vector3();
		}
		p2_Conflict = p1_Conflict->copy();
		p2_Conflict->set(0.0f, 0.0f, 0.0f);
		return p2_Conflict;
	} else {
		workingVector->set(p2_Conflict);
		return workingVector->subCopy(p1_Conflict);
	}
}

void Ray3D::alignTo(ewbik::math::Ray3D *target) {
	p1_Conflict->set(target->p1_Conflict);
	p2_Conflict->set(target->p2_Conflict);
}

void Ray3D::heading(std::vector<float> &newHead) {
	if (p2_Conflict == nullptr) {
		p2_Conflict = p1_Conflict->copy();
	}
	p2_Conflict->set(newHead);
	p2_Conflict->set(p1_Conflict);
}

void Ray3D::heading(Vector3 newHead) {
	if (p2_Conflict == nullptr) {
		p2_Conflict = p1_Conflict->copy();
	}
	p2_Conflict->set(p1_Conflict);
	p2_Conflict->add(newHead);
}

void Ray3D::getHeading(Vector3 setTo) {
	setTo->set(p2_Conflict);
	setTo->sub(this->p1_Conflict);
}

ewbik::math::Ray3D *Ray3D::get2DCopy() {
	return this->get2DCopy(ewbik::math::Ray3D::Z);
}

ewbik::math::Ray3D *Ray3D::get2DCopy(int collapseOnAxis) {
	ewbik::math::Ray3D *result = this->copy();
	if (collapseOnAxis == ewbik::math::Ray3D::X) {
		result->p1_Conflict->setX_(0);
		result->p2_Conflict->setX_(0);
	}
	if (collapseOnAxis == ewbik::math::Ray3D::Y) {
		result->p1_Conflict->setY_(0);
		result->p2_Conflict->setY_(0);
	}
	if (collapseOnAxis == ewbik::math::Ray3D::Z) {
		result->p1_Conflict->setZ_(0);
		result->p2_Conflict->setZ_(0);
	}

	return result;
}

Vector3 Ray3D::origin() {
	return p1_Conflict->copy();
}

float Ray3D::mag() {
	workingVector->set(p2_Conflict);
	return (workingVector->sub(p1_Conflict)).mag();
}

void Ray3D::mag(float newMag) {
	workingVector->set(p2_Conflict);
	Vector3 dir = workingVector->sub(p1_Conflict);
	dir->setMag(newMag);
	this->heading(dir);
}

float Ray3D::scaledProjection(Vector3 input) {
	workingVector->set(input);
	workingVector->sub(this->p1_Conflict);
	Vector3 heading = this->heading();
	float headingMag = heading->mag();
	float workingVectorMag = workingVector->mag();
	if (workingVectorMag == 0 || headingMag == 0) {
		return 0;
	} else {
		return (workingVector->dot(heading) / (headingMag * workingVectorMag)) * (workingVectorMag / headingMag);
	}
}

void Ray3D::div(float divisor) {
	p2_Conflict->sub(p1_Conflict);
	p2_Conflict->divide(divisor);
	p2_Conflict->add(p1_Conflict);
}

void Ray3D::multiply(float scalar) {
	p2_Conflict->sub(p1_Conflict);
	p2_Conflict->multiply(scalar);
	p2_Conflict->add(p1_Conflict);
}

Vector3 Ray3D::getMultipledBy(float scalar) {
	Vector3 result = this->heading();
	result->multiply(scalar);
	result->add(p1_Conflict);
	return result;
}

Vector3 Ray3D::getDivideddBy(float divisor) {
	Vector3 result = this->heading()->copy();
	result->multiply(divisor);
	result->add(p1_Conflict);
	return result;
}

Vector3 Ray3D::getScaledTo(float scale) {
	Vector3 result = this->heading()->copy();
	result->normalize();
	result->multiply(scale);
	result->add(p1_Conflict);
	return result;
}

void Ray3D::elongate(float amt) {
	Vector3 midPoint = p1_Conflict->addCopy(p2_Conflict).multCopy(0.5f);
	Vector3 p1Heading = p1_Conflict->subCopy(midPoint);
	Vector3 p2Heading = p2_Conflict->subCopy(midPoint);
	Vector3 p1Add = static_cast<Vector3 >(p1Heading->copy().normalize().multiply(amt));
	Vector3 p2Add = static_cast<Vector3 >(p2Heading->copy().normalize().multiply(amt));

	this->p1_Conflict->set(static_cast<Vector3 >(p1Heading->addCopy(p1Add).addCopy(midPoint)));
	this->p2_Conflict->set(static_cast<Vector3 >(p2Heading->addCopy(p2Add).addCopy(midPoint)));
}

ewbik::math::Ray3D *Ray3D::copy() {
	return new ewbik::math::Ray3D(this->p1_Conflict, this->p2_Conflict);
}

void Ray3D::reverse() {
	Vector3 temp = this->p1_Conflict;
	this->p1_Conflict = this->p2_Conflict;
	this->p2_Conflict = temp;
}

ewbik::math::Ray3D *Ray3D::getReversed() {
	return new ewbik::math::Ray3D(this->p2_Conflict, this->p1_Conflict);
}

ewbik::math::Ray3D *Ray3D::getRayScaledTo(float scalar) {
	return new ewbik::math::Ray3D(p1_Conflict, this->getScaledTo(scalar));
}

void Ray3D::pointWith(ewbik::math::Ray3D *r) {
	if (this->heading()->dot(r->heading()) < 0) {
		this->reverse();
	}
}

void Ray3D::pointWith(Vector3 heading) {
	if (this->heading()->dot(heading) < 0) {
		this->reverse();
	}
}

ewbik::math::Ray3D *Ray3D::getRayScaledBy(float scalar) {
	return new ewbik::math::Ray3D(p1_Conflict, this->getMultipledBy(scalar));
}

Vector3 Ray3D::setToInvertedTip(Vector3 vec) {
	vec->x = (p1_Conflict->x - p2_Conflict->x) + p1_Conflict->x;
	vec->y = (p1_Conflict->y - p2_Conflict->y) + p1_Conflict->y;
	vec->z = (p1_Conflict->z - p2_Conflict->z) + p1_Conflict->z;
	return vec;
}

void Ray3D::contractTo(float percent) {
	// contracts both ends of a ray toward its center such that the total length of
	// the ray
	// is the percent % of its current length;
	float halfPercent = 1 - ((1 - percent) / 2.0f);

	p1_Conflict = p1_Conflict->lerp(p2_Conflict, halfPercent); // )new Vector3(p1Tempx, p1Tempy, p1Tempz);
	p2_Conflict = p2_Conflict->lerp(p1_Conflict, halfPercent); // new Vector3(p2Tempx, p2Tempy, p2Tempz);
}

void Ray3D::translateTo(Vector3 newLocation) {
	workingVector->set(p2_Conflict);
	workingVector->sub(p1_Conflict);
	workingVector->add(newLocation);
	p2_Conflict->set(workingVector);
	p1_Conflict->set(newLocation);
}

void Ray3D::translateTipTo(Vector3 newLocation) {
	workingVector->set(newLocation);
	Vector3 transBy = workingVector->sub(p2_Conflict);
	this->translateBy(transBy);
}

void Ray3D::translateBy(Vector3 toAdd) {
	p1_Conflict->add(toAdd);
	p2_Conflict->add(toAdd);
}

void Ray3D::normalize() {
	this->mag(1);
}

Vector3 Ray3D::intercepts2D(ewbik::math::Ray3D *r) {
	Vector3 result = p1_Conflict->copy();

	float p0_x = this->p1_Conflict->x;
	float p0_y = this->p1_Conflict->y;
	float p1_x = this->p2_Conflict->x;
	float p1_y = this->p2_Conflict->y;

	float p2_x = r->p1_Conflict->x;
	float p2_y = r->p1_Conflict->y;
	float p3_x = r->p2_Conflict->x;
	float p3_y = r->p2_Conflict->y;

	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;
	s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;
	s2_y = p3_y - p2_y;

	float t;
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	return result->set(p0_x + (t * s1_x), p0_y + (t * s1_y), 0.0f);
}

Vector3 Ray3D::closestPointToSegment3D(ewbik::math::Ray3D *r) {
	Vector3 closestToThis = r->closestPointToRay3DStrict(this);
	return this->closestPointTo(closestToThis);
}

Vector3 Ray3D::closestPointToRay3D(ewbik::math::Ray3D *r) {
	Vector3 result = nullptr;

	workingVector->set(p2_Conflict);
	Vector3 u = workingVector->sub(this->p1_Conflict);
	workingVector->set(r->p2_Conflict);
	Vector3 v = workingVector->sub(r->p1_Conflict);
	workingVector->set(this->p1_Conflict);
	Vector3 w = workingVector->sub(r->p1_Conflict);
	float a = u->dot(u);
	float b = u->dot(v);
	float c = v->dot(v);
	float d = u->dot(w);
	float e = v->dot(w);
	float D = a * c - b * b;
	float sc;

	// compute the line parameters of the two closest points
	if (D < FLT_TRUE_MIN) {
		sc = 0.0f;
	} else {
		sc = (b * e - c * d) / D;
	}

	result = this->getRayScaledBy(sc)->p2;
	return result;
}

Vector3 Ray3D::closestPointToRay3DStrict(ewbik::math::Ray3D *r) {
	Vector3 result = nullptr;

	workingVector->set(p2_Conflict);
	Vector3 u = workingVector->sub(this->p1_Conflict);
	workingVector->set(r->p2_Conflict);
	Vector3 v = workingVector->sub(r->p1_Conflict);
	workingVector->set(this->p1_Conflict);
	Vector3 w = workingVector->sub(r->p1_Conflict);
	float a = u->dot(u); // always >= 0
	float b = u->dot(v);
	float c = v->dot(v); // always >= 0
	float d = u->dot(w);
	float e = v->dot(w);
	float D = a * c - b * b; // always >= 0
	float sc; // tc

	// compute the line parameters of the two closest points
	if (D < FLT_TRUE_MIN) {
		sc = 0.0f;
	} else {
		sc = (b * e - c * d) / D;
	}

	if (sc < 0) {
		result = this->p1_Conflict;
	} else if (sc > 1) {
		result = this->p2_Conflict;
	} else {
		result = this->getRayScaledBy(sc)->p2;
	}

	return result;
}

Vector3 Ray3D::closestPointToRay3DBounded(ewbik::math::Ray3D *r) {
	Vector3 result = nullptr;

	workingVector->set(p2_Conflict);
	Vector3 u = workingVector->sub(this->p1_Conflict);
	workingVector->set(r->p2_Conflict);
	Vector3 v = workingVector->sub(r->p1_Conflict);
	workingVector->set(this->p1_Conflict);
	Vector3 w = workingVector->sub(r->p1_Conflict);
	float a = u->dot(u); // always >= 0
	float b = u->dot(v);
	float c = v->dot(v); // always >= 0
	float d = u->dot(w);
	float e = v->dot(w);
	float D = a * c - b * b; // always >= 0
	float sc; // tc

	// compute the line parameters of the two closest points
	if (D < FLT_TRUE_MIN) {
		sc = 0.0f;
	} else {
		sc = (b * e - c * d) / D;
	}

	if (sc < 0) {
		result = nullptr;
	} else if (sc > 1) {
		result = nullptr;
	} else {
		result = this->getRayScaledBy(sc)->p2;
	}

	return result;
}

ewbik::math::Ray3D *Ray3D::getPerpendicular2D() {
	Vector3 heading = this->heading();
	workingVector->set(heading->x - 1.0f, heading->x, 0.0f);
	return new ewbik::math::Ray3D(this->p1_Conflict, workingVector->add(this->p1_Conflict));
}

Vector3 Ray3D::intercepts2DStrict(ewbik::math::Ray3D *r) {
	// will also return null if the intersection does not occur on the
	// line segment specified by the ray.
	Vector3 result = p1_Conflict->copy();

	// boolean over = false;
	float a1 = p2_Conflict->y - p1_Conflict->y;
	float b1 = p1_Conflict->x - p2_Conflict->x;
	float c1 = a1 * p1_Conflict->x + b1 * p1_Conflict->y;

	float a2 = r->p2_Conflict->y - r->p1_Conflict->y;
	float b2 = r->p1_Conflict->x - r->p2_Conflict->y;
	float c2 = a2 * +b2 * r->p1_Conflict->y;

	float det = a1 * b2 - a2 * b1;
	if (det == 0) {
		// Lines are parallel
		return nullptr;
	} else {
		result->setX_((b2 * c1 - b1 * c2) / det);
		result->setY_((a1 * c2 - a2 * c1) / det);
	}

	float position = result->dot(this->heading());
	if (position > 1 || position < 0) {
		return nullptr;
	}

	return result;
}

Vector3 Ray3D::intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc) {
	std::vector<float> uvw(3);
	return intersectsPlane(ta, tb, tc, uvw);
}

Vector3 Ray3D::intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc, std::vector<float> &uvw) {
	if (tta == nullptr) {
		tta = ta->copy();
		ttb = tb->copy();
		ttc = tc->copy();
	} else {
		tta->set(ta);
		ttb->set(tb);
		ttc->set(tc);
	}
	tta->sub(p1_Conflict);
	ttb->sub(p1_Conflict);
	ttc->sub(p1_Conflict);

	Vector3 result = static_cast<Vector3 >(planeIntersectTest(tta, ttb, ttc, uvw)->copy());
	return result->add(this->p1_Conflict);
}

void Ray3D::intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 result) {
	std::vector<float> uvw(3);
	result->set(intersectsPlane(ta, tb, tc, uvw));
}

bool Ray3D::intersectsTriangle(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 result) {
	std::vector<float> uvw(3);
	result->set(intersectsPlane(ta, tb, tc, uvw));
	return !std::isnan(uvw[0]) && !std::isnan(uvw[1]) && !std::isnan(uvw[2]) && !(uvw[0] < 0) && !(uvw[1] < 0) && !(uvw[2] < 0);
}

Vector3 Ray3D::planeIntersectTest(Vector3 ta, Vector3 tb, Vector3 tc, std::vector<float> &uvw) {
	if (u == nullptr) {
		u = tb->copy();
		v = tc->copy();
		dir = this->heading();
		w0 = p1_Conflict->copy();
		w0->set(0, 0, 0);
		I = p1_Conflict->copy();
	} else {
		u->set(tb);
		v->set(tc);
		n->set(0, 0, 0);
		dir->set(this->heading());
		w0->set(0, 0, 0);
	}
	// Vector3 w = new Vector3();
	float r, a, b;
	u->sub(ta);
	v->sub(ta);

	n = u->crossCopy(v);

	w0->sub(ta);
	a = -(n->dot(w0));
	b = n->dot(dir);
	r = a / b;
	I->set(0, 0, 0);
	I->set(dir);
	I->multiply(r);
	barycentric(ta, tb, tc, I, uvw);

	return static_cast<Vector3 >(I->copy());
}

int Ray3D::intersectsSphere(Vector3 sphereCenter, float radius, Vector3 S1, Vector3 S2) {
	Vector3 tp1 = p1_Conflict->subCopy(sphereCenter);
	Vector3 tp2 = p2_Conflict->subCopy(sphereCenter);
	int result = intersectsSphere(tp1, tp2, radius, S1, S2);
	S1->add(sphereCenter);
	S2->add(sphereCenter);
	return result;
}

int Ray3D::intersectsSphere(Vector3 rp1, Vector3 rp2, float radius, Vector3 S1, Vector3 S2) {
	Vector3 direction = static_cast<Vector3 >(rp2->subCopy(rp1));
	Vector3 e = static_cast<Vector3 >(direction->copy()); // e=ray.dir
	e->normalize(); // e=g/|g|
	Vector3 h = static_cast<Vector3 >(p1_Conflict->copy());
	h->set(0.0f, 0.0f, 0.0f);
	h = static_cast<Vector3 >(h->sub(rp1)); // h=r.o-c.M
	float lf = e->dot(h); // lf=e.h
	float radpow = radius * radius;
	float hdh = h->magSq();
	float lfpow = lf * lf;
	float s = radpow - hdh + lfpow; // s=r^2-h^2+lf^2
	if (s < 0.0f) {
		return 0; // no intersection points ?
	}
	s = MathUtils::sqrt(s); // s=sqrt(r^2-h^2+lf^2)

	int result = 0;
	if (lf < s) {
		if (lf + s >= 0) {
			s = -s; // swap S1 <-> S2}
			result = 1; // one intersection point
		}
	} else {
		result = 2; // 2 intersection points
	}

	S1->set(e->multCopy(lf - s));
	S1->add(rp1); // S1=A+e*(lf-s)
	S2->set(e->multCopy(lf + s));
	S2->add(rp1); // S2=A+e*(lf+s)
	return result;
}

void Ray3D::barycentric(Vector3 a, Vector3 b, Vector3 c, Vector3 p, std::vector<float> &uvw) {
	if (m == nullptr) {
		bc = b->copy();
		ca = c->copy();
		at = a->copy();
		bt = b->copy();
		ct = c->copy();
		pt = p->copy();
	} else {
		bc->set(b);
		ca->set(a);
		at->set(a);
		bt->set(b);
		ct->set(c);
		pt->set(p);
	}

	m = new Vector3((static_cast<Vector3 >(bc->subCopy(ct)))->crossCopy(static_cast<Vector3 >(ca->subCopy(at))));

	float nu;
	float nv;
	float ood;

	float x = MathUtils::abs(m->x);
	float y = MathUtils::abs(m->y);
	float z = MathUtils::abs(m->z);

	if (x >= y && x >= z) {
		nu = triArea2D(pt->y, pt->z, bt->y, bt->z, ct->y, ct->z);
		nv = triArea2D(pt->y, pt->z, ct->y, ct->z, at->y, at->z);
		ood = 1.0f / m->x;
	} else if (y >= x && y >= z) {
		nu = triArea2D(pt->x, pt->z, bt->x, bt->z, ct->x, ct->z);
		nv = triArea2D(pt->x, pt->z, ct->x, ct->z, at->x, at->z);
		ood = 1.0f / -m->y;
	} else {
		nu = triArea2D(pt->x, pt->y, bt->x, bt->y, ct->x, ct->y);
		nv = triArea2D(pt->x, pt->y, ct->x, ct->y, at->x, at->y);
		ood = 1.0f / m->z;
	}
	uvw[0] = nu * ood;
	uvw[1] = nv * ood;
	uvw[2] = 1.0f - uvw[0] - uvw[1];
}

std::wstring Ray3D::toString() {
	std::wstring result = L"sgRay " + L"\n" + L"(" + this->p1_Conflict->x + L" ->  " + this->p2_Conflict->x + L") \n " + L"(" + this->p1_Conflict->y + L" ->  " + this->p2_Conflict->y + L") \n " + L"(" + this->p1_Conflict->z + L" ->  " + this->p2_Conflict->z + L") \n ";
	return result;
}

void Ray3D::p1(Vector3 in) {
	this->p1_Conflict = in->copy();
}

void Ray3D::p2(Vector3 in) {
	this->p2_Conflict = in->copy();
}

float Ray3D::lerp(float p_a, float p_b, float p_t) {
	return (1 - p_t) * p_a + t * p_b;
}

Vector3 Ray3D::p2() {
	return p2_Conflict;
}

void Ray3D::set(Ray3D *r_r) {
	this->p1_Conflict->set(r_r->p1_Conflict);
	this->p2_Conflict->set(r_r->p2_Conflict);
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
