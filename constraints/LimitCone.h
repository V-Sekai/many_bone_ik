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
#include "../math/ik_transform.h"
#include "Kusudama.h"
#include "core/io/resource.h"
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

class IKKusudama;
class LimitCone : public Resource {
	GDCLASS(LimitCone, Resource);

public:
	Vector3 controlPoint;
	Vector3 radialPoint;

	// radius stored as  cosine to save on the acos call necessary for angleBetween.
private:
	double radiusCosine = 0;
	double radius = 0;
	double cushionRadius = 0;
	double cushionCosine = 0;
	double currentCushion = 1;

public:
	Ref<IKKusudama> parentKusudama;

	Vector3 tangentCircleCenterNext1;
	Vector3 tangentCircleCenterNext2;
	double tangentCircleRadiusNext = 0;
	double tangentCircleRadiusNextCos = 0;

	Vector3 cushionTangentCircleCenterNext1;
	Vector3 cushionTangentCircleCenterNext2;
	Vector3 cushionTangentCircleCenterPrevious1;
	Vector3 cushionTangentCircleCenterPrevious2;
	double cushionTangentCircleRadiusNext = 0;
	double cushionTangentCircleRadiusNextCos = 0;

	static const int BOUNDARY = 0;
	static const int CUSHION = 1;

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

	virtual ~LimitCone() {
		// delete controlPoint;
		// delete radialPoint;
		// delete parentKusudama;
		// delete tangentCircleCenterNext1;
		// delete tangentCircleCenterNext2;
		// delete cushionTangentCircleCenterNext1;
		// delete cushionTangentCircleCenterNext2;
		// delete cushionTangentCircleCenterPrevious1;
		// delete cushionTangentCircleCenterPrevious2;
	}

	LimitCone();

	LimitCone(Vector3 &direction, double rad, Ref<IKKusudama> attachedTo) {
		setControlPoint(direction);
		tangentCircleCenterNext1 = direction.getOrthogonal();
		tangentCircleCenterNext2 = Vector3::mult(tangentCircleCenterNext1, -1);

		this->radius = MAX(DBL_TRUE_MIN, rad);
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
	LimitCone(Vector3 direction, double rad, double cushion, Ref<IKKusudama> attachedTo) {
		setControlPoint(direction);
		tangentCircleCenterNext1 = direction->getOrthogonal();
		tangentCircleCenterNext2 = Vector3::mult(tangentCircleCenterNext1, -1);

		this->radius = MAX(DBL_TRUE_MIN, rad);
		this->radiusCosine = IKBoneChain::cos(radius);
		double adjustedCushion = MIN(1, MAX(0.001, cushion));
		this->cushionRadius = this->radius * adjustedCushion;
		this->cushionCosine = IKBoneChain::cos(cushionRadius);
		parentKusudama = attachedTo;
	}

	/**
	 *
	 * @param next
	 * @param input
	 * @param collisionPoint will be set to the rectified (if necessary) position of the input after accounting for collisions
	 * @return
	 */
	bool inBoundsFromThisToNext(Ref<LimitCone> next, Vector3 input, Vector3 collisionPoint) {
		bool isInBounds = false;
		Vector3 closestCollision = getClosestCollision(next, input);
		if (closestCollision != Vector3(NAN, NAN, NAN)) {
			/**
			 * getClosestCollision returns null if the point is already in bounds,
			 * so we set isInBounds to true.
			 */
			isInBounds = true;
			collisionPoint.x = input.x;
			collisionPoint.y = input.y;
			collisionPoint.z = input.z;
		} else {
			collisionPoint.x = closestCollision.x;
			collisionPoint.y = closestCollision.y;
			collisionPoint.z = closestCollision.z;
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
	Vector3 getClosestCollision(Ref<LimitCone> next, Vector3 input) {
		Vector3 result = getOnGreatTangentTriangle(next, input);
		if (result == Vector3(NAN, NAN, NAN)) {
			Vector<bool> inBounds = { false };
			result = closestPointOnClosestCone(next, input, inBounds);
		}
		return result;
	}

	Vector3 getClosestPathPoint(Ref<LimitCone> next, Vector3 input) {
		Vector3 result = getOnPathSequence(next, input);
		if (result == Vector3(NAN, NAN, NAN)) {
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
	bool determineIfInBounds(Ref<LimitCone> next, Vector3 input) {
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

	Vector3 getOnPathSequence(Ref<LimitCone> next, Vector3 input) {
		Vector3 c1xc2 = controlPoint.cross(next->controlPoint);
		double c1c2dir = input.dot(c1xc2);
		if (c1c2dir < 0.0) {
			Vector3 c1xt1 = controlPoint.cross(tangentCircleCenterNext1);
			Vector3 t1xc2 = tangentCircleCenterNext1.cross(next->controlPoint);
			if (input.dot(c1xt1) > 0 && input.dot(t1xc2) > 0) {
				Ref<Ray3D> tan1ToInput = new Ray3D(tangentCircleCenterNext1, input);
				Vector3 result;
				Vector3 tempVar;
				tan1ToInput->intersectsPlane(tempVar, controlPoint, next->controlPoint, result);
				return result.normalized();
			} else {
				return Vector3(NAN, NAN, NAN);
			}
		} else {
			Vector3 t2xc1 = tangentCircleCenterNext2.cross(controlPoint);
			Vector3 c2xt2 = next->controlPoint.cross(tangentCircleCenterNext2);
			if (input.dot(t2xc1) > 0 && input.dot(c2xt2) > 0) {
				Ref<Ray3D> tan2ToInput = memnew(Ray3D(tangentCircleCenterNext2, input));
				Vector3 result;
				Vector3 tempVar2;
				tan2ToInput->intersectsPlane(tempVar2, controlPoint, next->controlPoint, result);
				return result.normalized();
			} else {
				return Vector3(NAN, NAN, NAN);
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
	Vector3 getOnGreatTangentTriangle(Ref<LimitCone> next, Vector3 input) {
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

	Vector3 closestCone(Ref<LimitCone> next, Vector3 input) {
		if (input.dot(controlPoint) > input.dot(next->controlPoint)) {
			return this->controlPoint;
		} else {
			return next->controlPoint;
		}
	}

	/**
	 * returns null if no rectification is required.
	 * @param next
	 * @param input
	 * @param inBounds
	 * @return
	 */
	Vector3 closestPointOnClosestCone(Ref<LimitCone> next, Vector3 input, Vector<bool> &inBounds) {
		Vector3 closestToFirst = this->closestToCone(input, inBounds);
		if (inBounds[0]) {
			return closestToFirst;
		}
		Vector3 closestToSecond = next->closestToCone(input, inBounds);
		if (inBounds[0]) {
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

	/**
	 * returns null if no rectification is required.
	 * @param input
	 * @param inBounds
	 * @return
	 */
	Vector3 closestToCone(Vector3 input, Vector<bool> &inBounds) {
		if (input.dot(this->getControlPoint()) > this->getRadiusCosine()) {
			inBounds[0] = true;
			return nullptr;
		} else {
			Vector3 axis = this->getControlPoint().cross(input);
			Quaternion rotTo = Quaternion(axis, this->getRadius());
			Vector3 result = Basis(rotTo).xform(this->getControlPoint());
			inBounds[0] = false;
			return result;
		}
	}

	virtual void updateTangentHandles(Ref<LimitCone> next);

private:
	void updateTangentAndCushionHandles(Ref<LimitCone> next, int mode);

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
	void computeTriangles(Ref<LimitCone> next);

public:
	virtual Vector3 getControlPoint();
	void setControlPoint(Vector3 controlPoint) {
		this->controlPoint = controlPoint;
		this->controlPoint.normalize();
		if (this->parentKusudama.is_valid()) {
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

	virtual Ref<IKKusudama> getParentKusudama();
};
