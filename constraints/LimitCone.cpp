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

#include "LimitCone.h"

#include "Kusudama.h"

LimitCone::LimitCone() {
}

void LimitCone::updateTangentHandles(Ref<LimitCone> next) {
	this->controlPoint.normalize();
	updateTangentAndCushionHandles(next, BOUNDARY);
	updateTangentAndCushionHandles(next, CUSHION);
}

void LimitCone::updateTangentAndCushionHandles(Ref<LimitCone> next, int mode) {
	if (next.is_valid()) {
		double radA = this->_getRadius(mode);
		double radB = next->_getRadius(mode);

		Vector3 A = this->getControlPoint();
		Vector3 B = next->getControlPoint();

		Vector3 arcNormal = A.cross(B);

		/**
		 * There are an infinite number of circles co-tangent with A and B, every other
		 * one of which has a unique radius.
		 *
		 * However, we want the radius of our tangent circles to obey the following properties:
		 *   1) When the radius of A + B == 0, our tangent circle's radius should = 90.
		 *   	In other words, the tangent circle should span a hemisphere.
		 *   2) When the radius of A + B == 180, our tangent circle's radius should = 0.
		 *   	In other words, when A + B combined are capable of spanning the entire sphere,
		 *   	our tangentCircle should be nothing.
		 *
		 * Another way to think of this is -- whatever the maximum distance can be between the
		 * borders of A and B (presuming their centers are free to move about the circle
		 * but their radii remain constant), we want our tangentCircle's diameter to be precisely that distance,
		 * and so, our tangent circles radius should be precisely half of that distance.
		 */
		double tRadius = (static_cast<M_PI *>(-(radA + radB))) / 2;

		/**
		 * Once we have the desired radius for our tangent circle, we may find the solution for its
		 * centers (usually, there are two).
		 */
		double boundaryPlusTangentRadiusA = radA + tRadius;
		double boundaryPlusTangentRadiusB = radB + tRadius;

		// the axis of this cone, scaled to minimize its distance to the tangent  contact points.
		Vector3 scaledAxisA = Vector3::mult(A, std::cos(boundaryPlusTangentRadiusA));
		// a point on the plane running through the tangent contact points
		Basis tempVar(arcNormal, boundaryPlusTangentRadiusA);
		Vector3 planeDir1A = (&tempVar)->applyToCopy(A);
		// another point on the same plane
		Basis tempVar2(A, M_PI / 2);
		Vector3 planeDir2A = (&tempVar2)->applyToCopy(planeDir1A);

		Vector3 scaledAxisB = B * std::cos(boundaryPlusTangentRadiusB);
		// a point on the plane running through the tangent contact points
		Basis tempVar3(arcNormal, boundaryPlusTangentRadiusB);
		Vector3 planeDir1B = tempVar3.xform(B);
		// another point on the same plane
		Quaternion tempVar4(B, M_PI / 2);
		Vector3 planeDir2B = (&tempVar4)->applyToCopy(planeDir1B);

		// ray from scaled center of next cone to half way point between the circumference of this cone and the next cone.
		Ref<Ray3D> r1B = memnew(Ray3D(planeDir1B, scaledAxisB));
		Ref<Ray3D> r2B = memnew(Ray3D(planeDir1B, planeDir2B));

		r1B->elongate(99);
		r2B->elongate(99);

		Vector3 intersection1 = r1B->intersectsPlane(scaledAxisA, planeDir1A, planeDir2A);
		Vector3 intersection2 = r2B->intersectsPlane(scaledAxisA, planeDir1A, planeDir2A);

		Ref<Ray3D> intersectionRay = memnew(Ray3D(intersection1, intersection2));
		intersectionRay->elongate(99);

		Vector3 sphereIntersect1;
		Vector3 sphereIntersect2;
		Vector3 sphereCenter;
		intersectionRay->intersectsSphere(sphereCenter, 1.0f, sphereIntersect1, sphereIntersect2);

		this->setTangentCircleCenterNext1(sphereIntersect1, mode);
		this->setTangentCircleCenterNext2(sphereIntersect2, mode);
		this->setTangentCircleRadiusNext(tRadius, mode);
	}
	if (this->tangentCircleCenterNext1 == Vector3(NAN, NAN, NAN)) {
		this->tangentCircleCenterNext1 = getOrthogonal(controlPoint).normalize();
		this->cushionTangentCircleCenterNext1 = getOrthogonal(controlPoint).normalize();
	}
	if (tangentCircleCenterNext2 == Vector3(NAN, NAN, NAN)) {
		tangentCircleCenterNext2 = (tangentCircleCenterNext1 * -1).normalize();
		cushionTangentCircleCenterNext2 = (cushionTangentCircleCenterNext2 * -1).normalize();
	}
	if (next.is_valid()) {
		computeTriangles(next);
	}
}

void LimitCone::setTangentCircleRadiusNext(double rad, int mode) {
	if (mode == CUSHION) {
		this->cushionTangentCircleRadiusNext = rad;
		this->cushionTangentCircleRadiusNext = std::cos(cushionTangentCircleRadiusNextCos);
	}
	this->tangentCircleRadiusNext = rad;
	this->tangentCircleRadiusNextCos = std::cos(tangentCircleRadiusNext);
}

Vector3 LimitCone::getTangentCircleCenterNext1(int mode) {
	if (mode == CUSHION) {
		return cushionTangentCircleCenterNext1;
	}
	return tangentCircleCenterNext1;
}

double LimitCone::getTangentCircleRadiusNext(int mode) {
	if (mode == CUSHION) {
		return cushionTangentCircleRadiusNext;
	}
	return tangentCircleRadiusNext;
}

double LimitCone::getTangentCircleRadiusNextCos(int mode) {
	if (mode == CUSHION) {
		return cushionTangentCircleRadiusNextCos;
	}
	return tangentCircleRadiusNextCos;
}

Vector3 LimitCone::getTangentCircleCenterNext2(int mode) {
	if (mode == CUSHION) {
		return cushionTangentCircleCenterNext2;
	}
	return tangentCircleCenterNext2;
}

double LimitCone::_getRadius(int mode) {
	if (mode == CUSHION) {
		return cushionRadius;
	}
	return radius;
}

double LimitCone::_getRadiusCosine(int mode) {
	if (mode == CUSHION) {
		return cushionCosine;
	}
	return radiusCosine;
}

void LimitCone::computeTriangles(Ref<LimitCone> next) {
	firstTriangleNext[1] = this->tangentCircleCenterNext1.normalize();
	firstTriangleNext[0] = this->getControlPoint().normalize();
	firstTriangleNext[2] = next->getControlPoint().normalize();

	secondTriangleNext[1] = this->tangentCircleCenterNext2.normalize();
	secondTriangleNext[0] = this->getControlPoint().normalize();
	secondTriangleNext[2] = next->getControlPoint().normalize();
}

Vector3 LimitCone::getControlPoint() const {
	return controlPoint;
}

double LimitCone::getRadius() {
	return this->radius;
}

double LimitCone::getRadiusCosine() {
	return this->radiusCosine;
}

void LimitCone::setRadius(double radius) {
	this->radius = radius;
	this->radiusCosine = std::cos(radius);
	this->parentKusudama->constraintUpdateNotification();
}

double LimitCone::getCushionRadius() {
	return this->cushionRadius;
}

double LimitCone::getCushionCosine() {
	return this->cushionCosine;
}

void LimitCone::setCushionBoundary(double cushion) {
	double adjustedCushion = MIN(1, std::max(0.001, cushion));
	this->cushionRadius = this->radius * adjustedCushion;
	this->cushionCosine = std::cos(cushionRadius);
}

Ref<IKKusudama> LimitCone::getParentKusudama() {
	return parentKusudama;
}