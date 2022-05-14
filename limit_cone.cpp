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

#include "limit_cone.h"

#include "core/io/resource.h"
#include "core/object/ref_counted.h"

#include "kusudama.h"

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
		double tRadius = -(radA + radB) / 2;

		/**
		 * Once we have the desired radius for our tangent circle, we may find the solution for its
		 * centers (usually, there are two).
		 */
		double boundaryPlusTangentRadiusA = radA + tRadius;
		double boundaryPlusTangentRadiusB = radB + tRadius;

		// the axis of this cone, scaled to minimize its distance to the tangent  contact points.
		Vector3 scaledAxisA = A * cos(boundaryPlusTangentRadiusA);
		// a point on the plane running through the tangent contact points
		Basis tempVar(arcNormal, boundaryPlusTangentRadiusA);
		Vector3 planeDir1A = tempVar.xform(A);
		// another point on the same plane
		Basis tempVar2(A, Math_PI / 2);
		Vector3 planeDir2A = tempVar2.xform(planeDir1A);

		Vector3 scaledAxisB = B * cos(boundaryPlusTangentRadiusB);
		// a point on the plane running through the tangent contact points
		Basis tempVar3(arcNormal, boundaryPlusTangentRadiusB);
		Vector3 planeDir1B = tempVar3.xform(B);
		// another point on the same plane
		Basis tempVar4(B, Math_PI / 2);
		Vector3 planeDir2B = tempVar4.xform(planeDir1B);

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
		this->tangentCircleCenterNext1 = getOrthogonal(controlPoint).normalized();
		this->cushionTangentCircleCenterNext1 = getOrthogonal(controlPoint).normalized();
	}
	if (tangentCircleCenterNext2 == Vector3(NAN, NAN, NAN)) {
		tangentCircleCenterNext2 = (tangentCircleCenterNext1 * -1).normalized();
		cushionTangentCircleCenterNext2 = (cushionTangentCircleCenterNext2 * -1).normalized();
	}
	if (next.is_valid()) {
		computeTriangles(next);
	}
}

void LimitCone::setTangentCircleRadiusNext(double rad, int mode) {
	if (mode == CUSHION) {
		this->cushionTangentCircleRadiusNext = rad;
		this->cushionTangentCircleRadiusNext = cos(cushionTangentCircleRadiusNextCos);
	}
	this->tangentCircleRadiusNext = rad;
	this->tangentCircleRadiusNextCos = cos(tangentCircleRadiusNext);
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
	firstTriangleNext.write[1] = this->tangentCircleCenterNext1.normalized();
	firstTriangleNext.write[0] = this->getControlPoint().normalized();
	firstTriangleNext.write[2] = next->getControlPoint().normalized();

	secondTriangleNext.write[1] = this->tangentCircleCenterNext2.normalized();
	secondTriangleNext.write[0] = this->getControlPoint().normalized();
	secondTriangleNext.write[2] = next->getControlPoint().normalized();
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
	this->radiusCosine = cos(radius);
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
	this->cushionCosine = cos(cushionRadius);
}

Ref<IKKusudama> LimitCone::getParentKusudama() {
	return parentKusudama;
}

bool LimitCone::determineIfInBounds(Ref<LimitCone> next, Vector3 input) const {
	/**
	 * Procedure : Check if input is contained in this cone, or the next cone
	 * 	if it is, then we're finished and in bounds. otherwise,
	 * check if the point  is contained within the tangent radii,
	 * 	if it is, then we're out of bounds and finished, otherwise
	 * in the tangent triangles while still remaining outside of the tangent radii
	 * if it is, then we're finished and in bounds. otherwise, we're out of bounds.
	 */

	if (controlPoint.dot(input) >= radiusCosine) {
		return true;
	} else if (next != nullptr && next->controlPoint.dot(input) >= next->radiusCosine) {
		return true;
	} else {
		if (next == nullptr) {
			return false;
		}
		bool inTan1Rad = tangentCircleCenterNext1.dot(input) > tangentCircleRadiusNextCos;
		if (inTan1Rad) {
			return false;
		}
		bool inTan2Rad = tangentCircleCenterNext2.dot(input) > tangentCircleRadiusNextCos;
		if (inTan2Rad) {
			return false;
		}

		/*if we reach this point in the code, we are either on the path between two limit_cones, or on the path extending out from between them
		 * but outside of their radii.
		 * 	To determine which , we take the cross product of each control point with each tangent center.
		 * 		The direction of each of the resultant vectors will represent the normal of a plane.
		 * 		Each of these four planes define part of a boundary which determines if our point is in bounds.
		 * 		If the dot product of our point with the normal of any of these planes is negative, we must be out
		 * 		of bounds.
		 *
		 *	Older version of this code relied on a triangle intersection algorithm here, which I think is slightly less efficient on average
		 *	as it didn't allow for early termination. .
		 */

		Vector3 c1xc2 = controlPoint.cross(next->controlPoint);
		double c1c2dir = input.dot(c1xc2);

		if (c1c2dir < 0.0) {
			Vector3 c1xt1 = controlPoint.cross(tangentCircleCenterNext1);
			Vector3 t1xc2 = tangentCircleCenterNext1.cross(next->controlPoint);
			return input.dot(c1xt1) > 0 && input.dot(t1xc2) > 0;
		} else {
			Vector3 t2xc1 = tangentCircleCenterNext2.cross(controlPoint);
			Vector3 c2xt2 = next->controlPoint.cross(tangentCircleCenterNext2);
			return input.dot(t2xc1) > 0 && input.dot(c2xt2) > 0;
		}
	}
}

Vector3 LimitCone::getClosestPathPoint(Ref<LimitCone> next, Vector3 input) const {
	Vector3 result = getOnPathSequence(next, input);
	if (result == Vector3(NAN, NAN, NAN)) {
		result = closestCone(next, input);
	}
	return result;
}

Vector3 LimitCone::getClosestCollision(Ref<LimitCone> next, Vector3 input) {
	Vector3 result = getOnGreatTangentTriangle(next, input);
	if (result == Vector3(NAN, NAN, NAN)) {
		Vector<bool> in_bounds = { false };
		result = closestPointOnClosestCone(next, input, in_bounds);
	}
	return result;
}

bool LimitCone::inBoundsFromThisToNext(Ref<LimitCone> next, Vector3 input, Vector3 collision_point) {
	bool isInBounds = false;
	Vector3 closestCollision = getClosestCollision(next, input);
	if (closestCollision != Vector3(NAN, NAN, NAN)) {
		/**
		 * getClosestCollision returns null if the point is already in bounds,
		 * so we set isInBounds to true.
		 */
		isInBounds = true;
		collision_point.x = input.x;
		collision_point.y = input.y;
		collision_point.z = input.z;
	} else {
		collision_point.x = closestCollision.x;
		collision_point.y = closestCollision.y;
		collision_point.z = closestCollision.z;
	}
	return isInBounds;
}

Vector3 LimitCone::getOrthogonal(Vector3 p_in) {
	Vector3 result;
	float threshold = p_in.length() * 0.6f;
	if (threshold > 0.f) {
		if (Math::abs(p_in.x) <= threshold) {
			float inverse = 1.f / Math::sqrt(p_in.y * p_in.y + p_in.z * p_in.z);
			return result = Vector3(0.f, inverse * p_in.z, -inverse * p_in.y);
		} else if (Math::abs(p_in.y) <= threshold) {
			float inverse = 1.f / Math::sqrt(p_in.x * p_in.x + p_in.z * p_in.z);
			return result = Vector3(-inverse * p_in.z, 0.f, inverse * p_in.x);
		}
		float inverse = 1.f / Math::sqrt(p_in.x * p_in.x + p_in.y * p_in.y);
		return result = Vector3(inverse * p_in.y, -inverse * p_in.x, 0.f);
	}

	return result;
}

LimitCone::LimitCone(Vector3 direction, double rad, double cushion, Ref<IKKusudama> attachedTo) {
	setControlPoint(direction);
	tangentCircleCenterNext1 = LimitCone::getOrthogonal(direction);
	tangentCircleCenterNext2 = (tangentCircleCenterNext1 * -1);

	this->radius = MAX(DBL_TRUE_MIN, rad);
	this->radiusCosine = IKBoneChain::cos(radius);
	double adjustedCushion = MIN(1, MAX(0.001, cushion));
	this->cushionRadius = this->radius * adjustedCushion;
	this->cushionCosine = IKBoneChain::cos(cushionRadius);
	parentKusudama = attachedTo;
}
LimitCone::LimitCone(Vector3 &direction, double rad, Ref<IKKusudama> attachedTo) {
	setControlPoint(direction);
	tangentCircleCenterNext1 = direction.normalized();
	tangentCircleCenterNext2 = (tangentCircleCenterNext1 * -1);

	this->radius = MAX(DBL_TRUE_MIN, rad);
	this->radiusCosine = cos(radius);
	this->cushionRadius = this->radius;
	this->cushionCosine = this->radiusCosine;
	parentKusudama = attachedTo;
}

Vector3 LimitCone::getOnGreatTangentTriangle(Ref<LimitCone> next, Vector3 input) {
	Vector3 c1xc2 = controlPoint.cross(next->controlPoint);
	double c1c2dir = input.dot(c1xc2);
	if (c1c2dir < 0.0) {
		Vector3 c1xt1 = controlPoint.cross(tangentCircleCenterNext1);
		Vector3 t1xc2 = tangentCircleCenterNext1.cross(next->controlPoint);
		if (input.dot(c1xt1) > 0 && input.dot(t1xc2) > 0) {
			double toNextCos = input.dot(tangentCircleCenterNext1);
			if (toNextCos > tangentCircleRadiusNextCos) {
				Vector3 planeNormal = tangentCircleCenterNext1.cross(input);
				Quaternion rotateAboutBy = Quaternion(planeNormal, tangentCircleRadiusNext);
				return Basis(rotateAboutBy).xform(tangentCircleCenterNext1);
			} else {
				return input;
			}
		} else {
			return Vector3(NAN, NAN, NAN);
		}
	} else {
		Vector3 t2xc1 = tangentCircleCenterNext2.cross(controlPoint);
		Vector3 c2xt2 = next->controlPoint.cross(tangentCircleCenterNext2);
		if (input.dot(t2xc1) > 0 && input.dot(c2xt2) > 0) {
			if (input.dot(tangentCircleCenterNext2) > tangentCircleRadiusNextCos) {
				Vector3 planeNormal = tangentCircleCenterNext2.cross(input);
				Quaternion rotateAboutBy = Quaternion(planeNormal, tangentCircleRadiusNext);
				return Basis(rotateAboutBy).xform(tangentCircleCenterNext2);
			} else {
				return input;
			}
		} else {
			return Vector3(NAN, NAN, NAN);
		}
	}
}

Vector3 LimitCone::closestCone(Ref<LimitCone> next, Vector3 input) const {
	if (input.dot(controlPoint) > input.dot(next->controlPoint)) {
		return this->controlPoint;
	} else {
		return next->controlPoint;
	}
}

Vector3 LimitCone::closestPointOnClosestCone(Ref<LimitCone> next, Vector3 input, Vector<bool> &in_bounds) {
	Vector3 closestToFirst = this->closest_to_cone(input, in_bounds);
	if (in_bounds[0]) {
		return closestToFirst;
	}
	Vector3 closestToSecond = next->closest_to_cone(input, in_bounds);
	if (in_bounds[0]) {
		return closestToSecond;
	}
	double cosToFirst = input.dot(closestToFirst);
	double cosToSecond = input.dot(closestToSecond);

	if (cosToFirst > cosToSecond) {
		return closestToFirst;
	} else {
		return closestToSecond;
	}
}

Vector3 LimitCone::closest_to_cone(Vector3 input, Vector<bool> &in_bounds) {
	if (input.dot(this->getControlPoint()) > this->getRadiusCosine()) {
		in_bounds.write[0] = true;
		return Vector3(NAN, NAN, NAN);
	}
	if (Math::is_nan(input.x) || Math::is_nan(input.y) || Math::is_nan(input.z)) {
		return Vector3(NAN, NAN, NAN);
	}
	Vector3 axis = this->getControlPoint().cross(input).normalized();
	Quaternion rotTo = Quaternion(axis, this->getRadius());
	Vector3 axis_control_point = this->getControlPoint();
	Vector3 result = rotTo.xform(axis_control_point);
	in_bounds.write[0] = false;
	return result;
}

void LimitCone::setTangentCircleCenterNext1(Vector3 point, int mode) {
	if (mode == CUSHION) {
		this->cushionTangentCircleCenterNext1 = point;
	} else {
		this->tangentCircleCenterNext1 = point;
	}
}

void LimitCone::setTangentCircleCenterNext2(Vector3 point, int mode) {
	if (mode == CUSHION) {
		this->cushionTangentCircleCenterNext2 = point;
	} else {
		this->tangentCircleCenterNext2 = point;
	}
}

Vector3 LimitCone::getOnPathSequence(Ref<LimitCone> next, Vector3 input) const {
	Vector3 c1xc2 = getControlPoint().cross(next->controlPoint);
	double c1c2dir = input.dot(c1xc2);
	if (c1c2dir < 0.0) {
		Vector3 c1xt1 = getControlPoint().cross(tangentCircleCenterNext1);
		Vector3 t1xc2 = tangentCircleCenterNext1.cross(next->getControlPoint());
		if (input.dot(c1xt1) > 0.0f && input.dot(t1xc2) > 0.0f) {
			Ref<Ray3D> tan1ToInput = memnew(Ray3D(tangentCircleCenterNext1, input));
			Vector3 result = tan1ToInput->intersectsPlane(Vector3(0.0f, 0.0f, 0.0f), getControlPoint(), next->getControlPoint());
			return result.normalized();
		} else {
			return Vector3(NAN, NAN, NAN);
		}
	} else {
		Vector3 t2xc1 = tangentCircleCenterNext2.cross(controlPoint);
		Vector3 c2xt2 = next->getControlPoint().cross(tangentCircleCenterNext2);
		if (input.dot(t2xc1) > 0 && input.dot(c2xt2) > 0) {
			Ref<Ray3D> tan2ToInput = memnew(Ray3D(tangentCircleCenterNext2, input));
			Vector3 result = tan2ToInput->intersectsPlane(Vector3(0.0f, 0.0f, 0.0f), getControlPoint(), next->getControlPoint());
			return result.normalized();
		} else {
			return Vector3(NAN, NAN, NAN);
		}
	}
	return Vector3(NAN, NAN, NAN);
}