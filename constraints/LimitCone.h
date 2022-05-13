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

#pragma once

#define _USE_MATH_DEFINES
#include "Kusudama.h"
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

class AbstractLimitCone {
public:
	Vector3controlPoint;
	Vector3radialPoint;

	// radius stored as  cosine to save on the acos call necessary for angleBetween.
private:
	double radiusCosine = 0;
	double radius = 0;
	double cushionRadius = 0;
	double cushionCosine = 0;
	double currentCushion = 1;

public:
	IKKusudama *parentKusudama;

	Vector3tangentCircleCenterNext1;
	Vector3tangentCircleCenterNext2;
	double tangentCircleRadiusNext = 0;
	double tangentCircleRadiusNextCos = 0;

	Vector3cushionTangentCircleCenterNext1;
	Vector3cushionTangentCircleCenterNext2;
	Vector3cushionTangentCircleCenterPrevious1;
	Vector3cushionTangentCircleCenterPrevious2;
	double cushionTangentCircleRadiusNext = 0;
	double cushionTangentCircleRadiusNextCos = 0;

	static int BOUNDARY;
	static int CUSHION;

	// softness of 0 means completely hard.
	// any softness higher than 0f means that
	// as the softness value is increased
	// the is more penalized for moving
	// further from the center of the channel
	double softness = 0;

	/**
	 * a triangle where the [1] is th tangentCircleNext_n, and [0] and [2]
	 * are the points at which the tangent circle intersects this limitCone and the
	 * next limitCone
	 */
	Vector<Vector3> firstTriangleNext = { Vector3(), Vector3(), Vector3() };
	Vector<Vector3> secondTriangleNext = { Vector3(), Vector3(), Vector3() };

	virtual ~AbstractLimitCone() {
		delete controlPoint;
		delete radialPoint;
		delete parentKusudama;
		delete tangentCircleCenterNext1;
		delete tangentCircleCenterNext2;
		delete cushionTangentCircleCenterNext1;
		delete cushionTangentCircleCenterNext2;
		delete cushionTangentCircleCenterPrevious1;
		delete cushionTangentCircleCenterPrevious2;
	}

	AbstractLimitCone();

	AbstractLimitCone(Vector3 &direction, double rad, IKKusudama *attachedTo) {
		setControlPoint(direction);
		tangentCircleCenterNext1 = direction.getOrthogonal();
		tangentCircleCenterNext2 = SGVec_3d::mult(tangentCircleCenterNext1, -1);

		this->radius = std::max(DBL_TRUE_MIN, rad);
		this->radiusCosine = std::cos(radius);
		this->cushionRadius = this->radius;
		this->cushionCosine = this->radiusCosine;
		parentKusudama = attachedTo;
	}

	/**
	 *
	 * @param direction
	 * @param rad
	 * @param cushion range 0-1, how far toward the boundary to begin slowing down the rotation if soft constraints are enabled.
	 * Value of 1 creates a hard boundary. Value of 0 means it will always be the case that the closer a joint in the allowable region
	 * is to the boundary, the more any further rotation in the direction of that boundary will be avoided.
	 * @param attachedTo
	 */
	template <typename T1>
	AbstractLimitCone(Vector3 direction, double rad, double cushion, IKKusudama *attachedTo) {
		setControlPoint(direction);
		tangentCircleCenterNext1 = direction->getOrthogonal();
		tangentCircleCenterNext2 = SGVec_3d::mult(tangentCircleCenterNext1, -1);

		this->radius = std::max(DBL_TRUE_MIN, rad);
		this->radiusCosine = std::cos(radius);
		double adjustedCushion = std::min(1, std::max(0.001, cushion));
		this->cushionRadius = this->radius * adjustedCushion;
		this->cushionCosine = std::cos(cushionRadius);
		parentKusudama = attachedTo;
	}

	/**
	 *
	 * @param next
	 * @param input
	 * @param collisionPoint will be set to the rectified (if necessary) position of the input after accounting for collisions
	 * @return
	 */
	template <typename T1, typename T2>
	bool inBoundsFromThisToNext(AbstractLimitCone *next, Vector3 input, Vector3<T2> *collisionPoint) {
		bool isInBounds = false; // determineIfInBounds(next, input);
		Vector3closestCollision = getClosestCollision(next, input);
		if (closestCollision == nullptr) {
			/**
			 * getClosestCollision returns null if the point is already in bounds,
			 * so we set isInBounds to true.
			 */
			isInBounds = true;
			collisionPoint->x = input->x;
			collisionPoint->y = input->y;
			collisionPoint->z = input->z;
		} else {
			collisionPoint->x = closestCollision->x;
			collisionPoint->y = closestCollision->y;
			collisionPoint->z = closestCollision->z;
		}
		return isInBounds;
	}

	/**
	 *
	 * @param next
	 * @param input
	 * @return null if the input point is already in bounds, or the point's rectified position
	 * if the point was out of bounds.
	 */
	template <typename V>
	Vector3 *getClosestCollision(AbstractLimitCone *next, V input) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				Vector3result = getOnGreatTangentTriangle(next, input);
				if (result == nullptr) {
					Vector<bool> inBounds = { false };
					result = closestPointOnClosestCone(next, input, inBounds);
				}
				return result;
	}

	template <typename V>
	Vector3 *getClosestPathPoint(AbstractLimitCone *next, V input) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				Vector3result = getOnPathSequence(next, input);
				if (result == nullptr) {
					result = closestCone(next, input);
				}
				return result;
	}

	/**
	 * Determines if a ray emanating from the origin to given point in local space
	 * lies withing the path from this cone to the next cone. This function relies on
	 * an optimization trick for a performance boost, but the trick ruins everything
	 * if the input isn't normalized. So it is ABSOLUTELY VITAL
	 * that @param input have unit length in order for this function to work correctly.
	 * @param next
	 * @param input
	 * @return
	 */
	template <typename T1>
	bool determineIfInBounds(AbstractLimitCone *next, Vector3 input) {
		/**
		 * Procedure : Check if input is contained in this cone, or the next cone
		 * 	if it is, then we're finished and in bounds. otherwise,
		 * check if the point  is contained within the tangent radii,
		 * 	if it is, then we're out of bounds and finished, otherwise
		 * in the tangent triangles while still remaining outside of the tangent radii
		 * if it is, then we're finished and in bounds. otherwise, we're out of bounds.
		 */

		if (controlPoint->dot(input) >= radiusCosine) {
			return true;
		} else if (next != nullptr && next->controlPoint->dot(input) >= next->radiusCosine) {
			return true;
		} else {
			if (next == nullptr) {
				return false;
			}
			bool inTan1Rad = tangentCircleCenterNext1->dot(input) > tangentCircleRadiusNextCos;
			if (inTan1Rad) {
				return false;
			}
			bool inTan2Rad = tangentCircleCenterNext2->dot(input) > tangentCircleRadiusNextCos;
			if (inTan2Rad) {
				return false;
			}

			/*if we reach this point in the code, we are either on the path between two limitCones, or on the path extending out from between them
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

			Vector3 *c1xc2 = controlPoint->crossCopy(next->controlPoint);
			double c1c2dir = input->dot(c1xc2);

			if (c1c2dir < 0.0) {
				Vector3 *c1xt1 = controlPoint->crossCopy(tangentCircleCenterNext1);
				Vector3 *t1xc2 = tangentCircleCenterNext1->crossCopy(next->controlPoint);
				return input->dot(c1xt1) > 0 && input->dot(t1xc2) > 0;
			} else {
				Vector3 *t2xc1 = tangentCircleCenterNext2->crossCopy(controlPoint);
				Vector3 *c2xt2 = next->controlPoint->crossCopy(tangentCircleCenterNext2);
				return input->dot(t2xc1) > 0 && input->dot(c2xt2) > 0;
			}
		}
	}

	template <typename V>
	Vector3 *getOnPathSequence(AbstractLimitCone *next, V input) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				Vector3 *c1xc2 = controlPoint->crossCopy(next->controlPoint);
				double c1c2dir = input->dot(c1xc2);
				if (c1c2dir < 0.0) {
					Vector3 *c1xt1 = controlPoint->crossCopy(tangentCircleCenterNext1);
					Vector3 *t1xc2 = tangentCircleCenterNext1->crossCopy(next->controlPoint);
					if (input->dot(c1xt1) > 0 && input->dot(t1xc2) > 0) {
						sgRayd *tan1ToInput = new sgRayd(tangentCircleCenterNext1, input);
						Vector3result = new SGVec_3d();
						SGVec_3d tempVar(0, 0, 0);
						tan1ToInput->intersectsPlane(&tempVar, controlPoint, next->controlPoint, result);

						delete tan1ToInput;
						return result->normalize();
					} else {
						return nullptr;
					}
				} else {
					Vector3 *t2xc1 = tangentCircleCenterNext2->crossCopy(controlPoint);
					Vector3 *c2xt2 = next->controlPoint->crossCopy(tangentCircleCenterNext2);
					if (input->dot(t2xc1) > 0 && input->dot(c2xt2) > 0) {
						sgRayd *tan2ToInput = new sgRayd(tangentCircleCenterNext2, input);
						Vector3result = new SGVec_3d();
						SGVec_3d tempVar2(0, 0, 0);
						tan2ToInput->intersectsPlane(&tempVar2, controlPoint, next->controlPoint, result);

						delete tan2ToInput;
						return result->normalize();
					} else {
						return nullptr;
					}
				}
	}

	/**
	 *
	 * @param next
	 * @param input
	 * @return null if inapplicable for rectification. the original point if in bounds, or the point rectified to the closest boundary on the path sequence
	 * between two cones if the point is out of bounds and applicable for rectification.
	 */
	template <typename V>
	Vector3 *getOnGreatTangentTriangle(AbstractLimitCone *next, V input) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				Vector3 *c1xc2 = controlPoint->crossCopy(next->controlPoint);
				double c1c2dir = input->dot(c1xc2);
				if (c1c2dir < 0.0) {
					Vector3 *c1xt1 = controlPoint->crossCopy(tangentCircleCenterNext1);
					Vector3 *t1xc2 = tangentCircleCenterNext1->crossCopy(next->controlPoint);
					if (input->dot(c1xt1) > 0 && input->dot(t1xc2) > 0) {
						double toNextCos = input->dot(tangentCircleCenterNext1);
						if (toNextCos > tangentCircleRadiusNextCos) {
							Vector3planeNormal = tangentCircleCenterNext1->crossCopy(input);
							Rot *rotateAboutBy = new Rot(planeNormal, tangentCircleRadiusNext);

							delete rotateAboutBy;
							return rotateAboutBy->applyToCopy(tangentCircleCenterNext1);
						} else {
							return input;
						}
					} else {
						return nullptr;
					}
				} else {
					Vector3 *t2xc1 = tangentCircleCenterNext2->crossCopy(controlPoint);
					Vector3 *c2xt2 = next->controlPoint->crossCopy(tangentCircleCenterNext2);
					if (input->dot(t2xc1) > 0 && input->dot(c2xt2) > 0) {
						if (input->dot(tangentCircleCenterNext2) > tangentCircleRadiusNextCos) {
							Vector3planeNormal = tangentCircleCenterNext2->crossCopy(input);
							Rot *rotateAboutBy = new Rot(planeNormal, tangentCircleRadiusNext);

							delete rotateAboutBy;
							return rotateAboutBy->applyToCopy(tangentCircleCenterNext2);
						} else {
							return input;
						}
					} else {
						return nullptr;
					}
				}
	}

	template <typename V>
	Vector3 *closestCone(AbstractLimitCone *next, V input) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				if (input->dot(controlPoint) > input->dot(next->controlPoint)) {
					return this->controlPoint->copy();
				} else {
					return next->controlPoint->copy();
				}
	}

	/**
	 * returns null if no rectification is required.
	 * @param next
	 * @param input
	 * @param inBounds
	 * @return
	 */
	template <typename V>
	Vector3 *closestPointOnClosestCone(AbstractLimitCone *next, V input, Vector<bool> &inBounds) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				Vector3closestToFirst = this->closestToCone(input, inBounds);
				if (inBounds[0]) {
					return closestToFirst;
				}
				Vector3closestToSecond = next->closestToCone(input, inBounds);
				if (inBounds[0]) {
					return closestToSecond;
				}

				double cosToFirst = input->dot(closestToFirst);
				double cosToSecond = input->dot(closestToSecond);

				if (cosToFirst > cosToSecond) {
					return closestToFirst;
				} else {
					return closestToSecond;
				}
	}

	/**
	 * returns null if no rectification is required.
	 * @param input
	 * @param inBounds
	 * @return
	 */
	Vector3 closestToCone(Vector3 input, Vector<bool> &inBounds) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");

				if (input->dot(this->getControlPoint()) > this->getRadiusCosine()) {
					inBounds[0] = true;
					return nullptr;
				} else {
					Vector3axis = this->getControlPoint()->crossCopy(input);
					Rot *rotTo = new Rot(axis, this->getRadius());
					s
							Vector3result = rotTo->applyToCopy(this->getControlPoint());
					inBounds[0] = false;

					delete rotTo;
					return result;
				}
	}

	virtual void updateTangentHandles(AbstractLimitCone *next);

private:
	void updateTangentAndCushionHandles(AbstractLimitCone *next, int mode);

	void setTangentCircleCenterNext1(Vector3 point, int mode) {
		if (mode == CUSHION) {
			this->cushionTangentCircleCenterNext1 = point;
		} else {
			this->tangentCircleCenterNext1 = point;
		}
	}
	void setTangentCircleCenterNext2(Vector3 point, int mode) {
		if (mode == CUSHION) {
			this->cushionTangentCircleCenterNext2 = point;
		} else {
			this->tangentCircleCenterNext2 = point;
		}
	}

	void setTangentCircleRadiusNext(double rad, int mode);
	/**
	 * for internal and rendering use only. Avoid modifying any values in the resulting object,
	 * which is returned by reference.
	 * @param mode
	 * @return
	 */
protected:
	virtual Vector3getTangentCircleCenterNext1(int mode);

	virtual double getTangentCircleRadiusNext(int mode);

	virtual double getTangentCircleRadiusNextCos(int mode);

	/**
	 * for internal and rendering use only. Avoid modifying any values in the resulting object,
	 * which is returned by reference.
	 * @param mode
	 * @return
	 */
	virtual Vector3getTangentCircleCenterNext2(int mode);

	virtual double _getRadius(int mode);

	virtual double _getRadiusCosine(int mode);

private:
	void computeTriangles(AbstractLimitCone *next);

public:
	virtual Vector3 *getControlPoint();

	template <typename T1>
	void setControlPoint(Vector3 controlPoint) {
		this->controlPoint = controlPoint->copy();
		this->controlPoint->normalize();
		if (this->parentKusudama != nullptr) {
			this->parentKusudama->constraintUpdateNotification();
		}
	}

	virtual double getRadius();

	virtual double getRadiusCosine();

	virtual void setRadius(double radius);

	virtual double getCushionRadius();

	virtual double getCushionCosine();

	/**
	 * @param cushion range 0-1, how far toward the boundary to begin slowing down the rotation if soft constraints are enabled.
	 * Value of 1 creates a hard boundary. Value of 0 means it will always be the case that the closer a joint in the allowable region
	 * is to the boundary, the more any further rotation in the direction of that boundary will be avoided.
	 */
	virtual void setCushionBoundary(double cushion);

	virtual IKKusudama *getParentKusudama();

private:
	int BOUNDARY = 0;
	int CUSHION = 1;

	AbstractLimitCone() {
	}

	void updateTangentHandles(AbstractLimitCone *next) {
		this->controlPoint->normalize();
		updateTangentAndCushionHandles(next, BOUNDARY);
		updateTangentAndCushionHandles(next, CUSHION);
	}

	void updateTangentAndCushionHandles(AbstractLimitCone *next, int mode) {
		if (next != nullptr) {
			double radA = this->_getRadius(mode);
			double radB = next->_getRadius(mode);

			Vector3 *A = this->getControlPoint()->copy();
			Vector3 *B = next->getControlPoint()->copy();

			Vector3 *arcNormal = A->crossCopy(B);

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
			Vector3 *scaledAxisA = SGVec_3d::mult(A, std::cos(boundaryPlusTangentRadiusA));
			// a point on the plane running through the tangent contact points
			Rot tempVar(arcNormal, boundaryPlusTangentRadiusA);
			Vector3 *planeDir1A = (&tempVar)->applyToCopy(A);
			// another point on the same plane
			Rot tempVar2(A, M_PI / 2);
			Vector3 *planeDir2A = (&tempVar2)->applyToCopy(planeDir1A);

			Vector3 *scaledAxisB = Vector3::mult(B, std::cos(boundaryPlusTangentRadiusB));
			// a point on the plane running through the tangent contact points
			Rot tempVar3(arcNormal, boundaryPlusTangentRadiusB);
			Vector3 *planeDir1B = (&tempVar3)->applyToCopy(B);
			// another poiint on the same plane
			Rot tempVar4(B, M_PI / 2);
			Vector3 *planeDir2B = (&tempVar4)->applyToCopy(planeDir1B);

			// ray from scaled center of next cone to half way point between the circumference of this cone and the next cone.
			sgRayd *r1B = new sgRayd(planeDir1B, scaledAxisB);
			sgRayd *r2B = new sgRayd(planeDir1B, planeDir2B);

			r1B->elongate(99);
			r2B->elongate(99);

			Vector3 *intersection1 = r1B->intersectsPlane(scaledAxisA, planeDir1A, planeDir2A);
			Vector3 *intersection2 = r2B->intersectsPlane(scaledAxisA, planeDir1A, planeDir2A);

			sgRayd *intersectionRay = new sgRayd(intersection1, intersection2);
			intersectionRay->elongate(99);

			Vector3 *sphereIntersect1 = new SGVec_3d();
			Vector3 *sphereIntersect2 = new SGVec_3d();
			Vector3 *sphereCenter = new SGVec_3d();
			intersectionRay->intersectsSphere(sphereCenter, 1.0f, sphereIntersect1, sphereIntersect2);

			this->setTangentCircleCenterNext1(sphereIntersect1, mode);
			this->setTangentCircleCenterNext2(sphereIntersect2, mode);
			this->setTangentCircleRadiusNext(tRadius, mode);

			delete intersectionRay;
			delete r2B;
			delete r1B;
		}
		if (this->tangentCircleCenterNext1 == nullptr) {
			this->tangentCircleCenterNext1 = controlPoint->getOrthogonal().normalize();
			this->cushionTangentCircleCenterNext1 = controlPoint->getOrthogonal().normalize();
		}
		if (tangentCircleCenterNext2 == nullptr) {
			tangentCircleCenterNext2 = SGVec_3d::mult(tangentCircleCenterNext1, -1).normalize();
			cushionTangentCircleCenterNext2 = SGVec_3d::mult(cushionTangentCircleCenterNext2, -1).normalize();
		}
		if (next != nullptr) {
			computeTriangles(next);
		}
	}

	void setTangentCircleRadiusNext(double rad, int mode) {
		if (mode == CUSHION) {
			this->cushionTangentCircleRadiusNext = rad;
			this->cushionTangentCircleRadiusNext = std::cos(cushionTangentCircleRadiusNextCos);
		}
		this->tangentCircleRadiusNext = rad;
		this->tangentCircleRadiusNextCos = std::cos(tangentCircleRadiusNext);
	}

	Vector3 *getTangentCircleCenterNext1(int mode) {
		if (mode == CUSHION) {
			return cushionTangentCircleCenterNext1;
		}
		return tangentCircleCenterNext1;
	}

	double getTangentCircleRadiusNext(int mode) {
		if (mode == CUSHION) {
			return cushionTangentCircleRadiusNext;
		}
		return tangentCircleRadiusNext;
	}

	double getTangentCircleRadiusNextCos(int mode) {
		if (mode == CUSHION) {
			return cushionTangentCircleRadiusNextCos;
		}
		return tangentCircleRadiusNextCos;
	}

	Vector3 *getTangentCircleCenterNext2(int mode) {
		if (mode == CUSHION) {
			return cushionTangentCircleCenterNext2;
		}
		return tangentCircleCenterNext2;
	}

	double _getRadius(int mode) {
		if (mode == CUSHION) {
			return cushionRadius;
		}
		return radius;
	}

	double _getRadiusCosine(int mode) {
		if (mode == CUSHION) {
			return cushionCosine;
		}
		return radiusCosine;
	}

	void computeTriangles(AbstractLimitCone *next) {
		firstTriangleNext[1] = this->tangentCircleCenterNext1->normalize();
		firstTriangleNext[0] = this->getControlPoint()->normalize();
		firstTriangleNext[2] = next->getControlPoint()->normalize();

		secondTriangleNext[1] = this->tangentCircleCenterNext2->normalize();
		secondTriangleNext[0] = this->getControlPoint()->normalize();
		secondTriangleNext[2] = next->getControlPoint()->normalize();
	}

	Vector<Vector3> *getControlPoint() {
		return controlPoint;
	}

	double getRadius() {
		return this->radius;
	}

	double getRadiusCosine() {
		return this->radiusCosine;
	}

	void setRadius(double radius) {
		this->radius = radius;
		this->radiusCosine = std::cos(radius);
		this->parentKusudama->constraintUpdateNotification();
	}

	double getCushionRadius() {
		return this->cushionRadius;
	}

	double getCushionCosine() {
		return this->cushionCosine;
	}

	void setCushionBoundary(double cushion) {
		double adjustedCushion = MIN(1, std::max(0.001, cushion));
		this->cushionRadius = this->radius * adjustedCushion;
		this->cushionCosine = std::cos(cushionRadius);
	}

	IKKusudama *getParentKusudama() {
		return parentKusudama;
	}
};
