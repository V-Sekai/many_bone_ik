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
#include "Constraint.h"
#include "LimitCone.h"
#include "vectorhelper.h"
#include <cmath>
#include <type_traits>
#include <vector>

class IKKusudama : public Constraint {
	GDCLASS(Constraint, Constraint);

public:
	static const double TAU;
	static const double PI;

protected:
	AbstractAxes *limitingAxes_Conflict;
	double painfullness = 0;

	/**
	 * An array containing all of the Kusudama's limitCones. The kusudama is built up
	 * with the expectation that any limitCone in the array is connected to the cone at the previous element in the array,
	 * and the cone at the next element in the array.
	 */
	Vector<AbstractLimitCone *> limitCones = Vector<AbstractLimitCone *>();

	/**
	 * Defined as some Angle in radians about the limitingAxes Y axis, 0 being equivalent to the
	 * limitingAxes Z axis.
	 */
	double minAxialAngle_Conflict = M_PI;
	/**
	 * Defined as some Angle in radians about the limitingAxes Y axis, 0 being equivalent to the
	 * minAxialAngle
	 */
	double range = M_PI * 3;

	bool orientationallyConstrained = false;
	bool axiallyConstrained = false;

	// for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	// per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.
	real_t strength = 1;

	AbstractBone *attachedTo_Conflict;

public:
	virtual ~IKKusudama() {
		delete limitingAxes_Conflict;
		delete attachedTo_Conflict;
		delete boneRay;
		delete constrainedRay;
	}

	IKKusudama();

	IKKusudama(AbstractBone *forBone);

	virtual void constraintUpdateNotification();

	/**
	 * This function should be called after you've set all of the Limiting Cones
	 * for this Kusudama. It will orient the axes relative to which constrained rotations are computed
	 * so as to minimize the potential for undesirable twist rotations due to antipodal singularities.
	 *
	 * In general, auto-optimization attempts to point the y-component of the constraint
	 * axes in the direction that places it within an oreintation allowed by the constraint,
	 * and roughly as far as possible from any orientations not allowed by the constraint.
	 */
	virtual void optimizeLimitingAxes();

	virtual void updateTangentRadii();

	sgRayd *boneRay = new sgRayd(new SGVec_3d(), new SGVec_3d());
	sgRayd *constrainedRay = new sgRayd(new SGVec_3d(), new SGVec_3d());

	/**
	 * Snaps the bone this Kusudama is constraining to be within the Kusudama's orientational and axial limits.
	 */
	void snapToLimits() override;

	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param toSet
	 */
	virtual void setAxesToSnapped(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfAngleDampen);

	virtual void setAxesToReturnfulled(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfReturnfullness, double angleReturnfullness);

	/**
	 * A value between (ideally between 0 and 1) dictating
	 * how much the bone to which this kusudama belongs
	 * prefers to be away from the edges of the kusudama
	 * if it can. This is useful for avoiding unnatural poses,
	 * as the kusudama will push bones back into their more
	 * "comfortable" regions. Leave this value at its default of
	 * 0 unless you empircal observations show you need it.
	 * Setting this value to anything higher than 0.4 is probably overkill
	 * in most situations.
	 *
	 * @param amt
	 */
	virtual void setPainfullness(double amt);

	/**
	 * @return A value between (ideally between 0 and 1) dictating
	 * how much the bone to which this kusudama belongs
	 * prefers to be away from the edges of the kusudama
	 * if it can.
	 */
	virtual double getPainfullness();

	bool isInLimits_(Vector3 globalPoint) {
		Vector<double> inBounds = { 1 };
		Vector3 *inLimits = this->pointInLimits(globalPoint, inBounds, AbstractLimitCone::BOUNDARY);
		return inBounds[0] > 0;
	}

	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param toSet
	 */
	virtual void setAxesToSoftOrientationSnap(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfAngleDampen);

	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param toSet
	 */
	virtual void setAxesToOrientationSnap(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfAngleDampen);

	virtual bool isInOrientationLimits(AbstractAxes *globalAxes, AbstractAxes *limitingAxes);

	/**
	 * Kusudama constraints decompose the bone orientation into a swing component, and a twist component.
	 * The "Swing" component is the final direction of the bone. The "Twist" component represents how much
	 * the bone is rotated about its own final direction. Where limit cones allow you to constrain the "Swing"
	 * component, this method lets you constrain the "twist" component.
	 *
	 * @param minAnlge some angle in radians about the major rotation frame's y-axis to serve as the first angle within the range that the bone is allowed to twist.
	 * @param inRange some angle in radians added to the minAngle. if the bone's local Z goes maxAngle radians beyond the minAngle, it is considered past the limit.
	 * This value is always interpreted as being in the positive direction. For example, if this value is -PI/2, the entire range from minAngle to minAngle + 3PI/4 is
	 * considered valid.
	 */
	virtual void setAxialLimits(double minAngle, double inRange);

	// protected CartesianAxes limitLocalAxes;

	/**
	 *
	 * @param toSet
	 * @param limitingAxes
	 * @return radians of twist required to snap bone into twist limits (0 if bone is already in twist limits)
	 */
	virtual double snapToTwistLimits(AbstractAxes *toSet, AbstractAxes *limitingAxes);

	virtual double angleToTwistCenter(AbstractAxes *toSet, AbstractAxes *limitingAxes);

	virtual bool inTwistLimits(AbstractAxes *boneAxes, AbstractAxes *limitingAxes);

	virtual double signedAngleDifference(double minAngle, double __super);

	/**
	 * Given a point (in global coordinates), checks to see if a ray can be extended from the Kusudama's
	 * origin to that point, such that the ray in the Kusudama's reference frame is within the range allowed by the Kusudama's
	 * coneLimits.
	 * If such a ray exists, the original point is returned (the point is within the limits).
	 * If it cannot exist, the tip of the ray within the kusudama's limits that would require the least rotation
	 * to arrive at the input point is returned.
	 * @param inPoint the point to test.
	 * @param inBounds should be an array with at least 2 elements. The first element will be set to  a number from -1 to 1 representing the point's distance from the boundary, 0 means the point is right on
	 * the boundary, 1 means the point is within the boundary and on the path furthest from the boundary. any negative number means
	 * the point is outside of the boundary, but does not signify anything about how far from the boundary the point is.
	 * The second element will be given a value corresponding to the limit cone whose bounds were exceeded. If the bounds were exceeded on a segment between two limit cones,
	 * this value will be set to a non-integer value between the two indices of the limitcone comprising the segment whose bounds were exceeded.
	 * @return the original point, if it's in limits, or the closest point which is in limits.
	 */
	Vector3 *pointInLimits(Vector3 inPoint, Vector<double> &inBounds, int mode) {
				static_assert(std::is_base_of<math.doubleV.Vector3<?>, V>::value, L"V must inherit from math.doubleV.Vector3<?>");
				Vector3 *point = inPoint->copy();
				point->normalize();

				inBounds[0] = -1;
				Vector3 *closestCollisionPoint = nullptr;
				double closestCos = -2;
				Vector<bool> boundHint = { false };

				for (int i = 0; i < limitCones.size(); i++) {
					AbstractLimitCone *cone = limitCones[i];
					Vector3 *collisionPoint = inPoint->copy();
					collisionPoint->set(0, 0, 0);
					collisionPoint = cone->closestToCone(point, boundHint);
					if (collisionPoint == nullptr) {
						inBounds[0] = 1;
						return point;
					} else {
						double thisCos = collisionPoint->dot(point);
						if (closestCollisionPoint == nullptr || thisCos > closestCos) {
							closestCollisionPoint = collisionPoint;
							closestCos = thisCos;
						}
					}
				}
				if (inBounds[0] == -1) {
					for (int i = 0; i < limitCones.size() - 1; i++) {
						AbstractLimitCone *currCone = limitCones[i];
						AbstractLimitCone *nextCone = limitCones[i + 1];
						Vector3 *collisionPoint = inPoint->copy();
						collisionPoint->set(0, 0, 0);
						collisionPoint = currCone->getOnGreatTangentTriangle(nextCone, point);
						if (collisionPoint != nullptr) {
							double thisCos = collisionPoint->dot(point);
							if (thisCos == 1) {
								inBounds[0] = 1;
								closestCollisionPoint = point;
								return point;
							} else if (thisCos > closestCos) {
								closestCollisionPoint = collisionPoint;
								closestCos = thisCos;
							}
						}
					}
				}

				return closestCollisionPoint;
	}

	Vector3 *pointOnPathSequence(Vector3 inPoint, AbstractAxes *limitingAxes) {
		double closestPointDot = 0;
		Vector3 *point = limitingAxes->getLocalOf(inPoint);
		point->normalize();
		Vector3 *result = static_cast<Vector3 *>(point->copy());

		if (limitCones.size() == 1) {
			result->set(limitCones[0]->controlPoint);
		} else {
			for (int i = 0; i < limitCones.size() - 1; i++) {
				AbstractLimitCone *nextCone = limitCones[i + 1];
				Vector3 *closestPathPoint = limitCones[i]->getClosestPathPoint(nextCone, point);
				double closeDot = closestPathPoint->dot(point);
				if (closeDot > closestPointDot) {
					result->set(closestPathPoint);
					closestPointDot = closeDot;
				}
			}
		}

		return limitingAxes->getGlobalOf(result);
	}

	// public double softLimit

	virtual AbstractBone *attachedTo();

	/**
	 * Add a LimitCone to the Kusudama.
	 * @param newPoint where on the Kusudama to add the LimitCone (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
	 * @param radius the radius of the limitCone
	 * @param previous the LimitCone adjacent to this one (may be null if LimitCone is not supposed to be between two existing LimitCones)
	 * @param next the other LimitCone adjacent to this one (may be null if LimitCone is not supposed to be between two existing LimitCones)
	 */
	virtual void addLimitCone(SGVec_3d *newPoint, double radius, AbstractLimitCone *previous, AbstractLimitCone *next);

	virtual void removeLimitCone(AbstractLimitCone *limitCone);

	template <typename T1>
	AbstractLimitCone *createLimitConeForIndex(int insertAt, Vector3<T1> *newPoint, double radius) = 0;

	/**
	 * Adds a LimitCone to the Kusudama. LimitCones are reach cones which can be arranged sequentially. The Kusudama will infer
	 * a smooth path leading from one LimitCone to the next.
	 *
	 * Using a single LimitCone is functionally equivalent to a classic reachCone constraint.
	 *
	 * @param insertAt the intended index for this LimitCone in the sequence of LimitCones from which the Kusudama will infer a path. @see IK.IKKusudama.limitCones limitCones array.
	 * @param newPoint where on the Kusudama to add the LimitCone (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
	 * @param radius the radius of the limitCone
	 */
	virtual void addLimitConeAtIndex(int insertAt, SGVec_3d *newPoint, double radius);

	virtual double toTau(double angle);

	virtual double mod(double x, double y);

	/**
	 * @return the limitingAxes of this Kusudama (these are just its parentBone's majorRotationAxes)
	 */
	template <typename A>
	A limitingAxes() {
		static_assert(std::is_base_of<math.doubleV.AbstractAxes, A>::value, L"A must inherit from math.doubleV.AbstractAxes");

		// if(inverted) return inverseLimitingAxes;
		return static_cast<A>(limitingAxes_Conflict);
	}

	/**
	 *
	 * @return the lower bound on the axial constraint
	 */
	virtual double minAxialAngle();

	virtual double maxAxialAngle();

	/**
	 * the upper bound on the axial constraint in absolute terms
	 * @return
	 */
	virtual double absoluteMaxAxialAngle();

	virtual bool isAxiallyConstrained();

	virtual bool isOrientationallyConstrained();

	virtual void disableOrientationalLimits();

	virtual void enableOrientationalLimits();

	virtual void toggleOrientationalLimits();

	virtual void disableAxialLimits();

	virtual void enableAxialLimits();

	virtual void toggleAxialLimits();

	bool isEnabled() override;

	void disable() override;

	void enable() override;

	double unitHyperArea = 2 * std::pow(M_PI, 2);
	double unitArea = 4 * M_PI;

	/**
	 * TODO: // this functionality is not yet fully implemented It always returns an overly simplistic representation
	 * not in line with what is described below.
	 *
	 * @return an (approximate) measure of the amount of rotational
	 * freedom afforded by this kusudama, with 0 meaning no rotational
	 * freedom, and 1 meaning total unconstrained freedom.
	 *
	 * This is approximately computed as a ratio between the orientations the bone can be in
	 * vs the orientations it cannot be in. Note that unfortunately this function double counts
	 * the orientations a bone can be in between two limit cones in a sequence if those limit
	 * cones intersect with a previous sequence.
	 */
	double getRotationalFreedom() override;

	double rotationalFreedom = 1;

protected:
	virtual void updateRotationalFreedom();

	/**
	 * attaches the Kusudama to the BoneExample. If the
	 * kusudama has its own limiting axes specified,
	 * replaces the bone's major rotation
	 * axes with the Kusudamas limiting axes.
	 *
	 * otherwise, this function will set the kusudama's
	 * limiting axes to the major rotation axes specified by the bone.
	 *
	 * @param forBone the bone to which to attach this Kusudama.
	 */
public:
	virtual void attachTo(AbstractBone *forBone);

	/**for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	 per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.
	 @param strength a value between 0 and 1. Any other value will be clamped to this range.
	 **/
	virtual void setStrength(double newStrength);

	/**for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	 per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.**/
	virtual double getStrength();

	virtual Vector < ? extends AbstractLimitCone > getLimitCones();
};

IKKusudama::IKKusudama() {
}

IKKusudama::IKKusudama(AbstractBone *forBone) {
	this->attachedTo_Conflict = forBone;
	this->limitingAxes_Conflict = forBone->getMajorRotationAxes();
	this->attachedTo_Conflict->addConstraint(this);
	this->enable();
}

void IKKusudama::constraintUpdateNotification() {
	this->updateTangentRadii();
	this->updateRotationalFreedom();
}

void IKKusudama::optimizeLimitingAxes() {
	AbstractAxes *originalLimitingAxes = limitingAxes_Conflict->getGlobalCopy();
	Vector<Vector3> directions;
	if (getLimitCones().size() == 1) {
		directions.push_back((limitCones[0]->getControlPoint())->copy());
	} else {
		for (int i = 0; i < getLimitCones().size() - 1; i++) {
			Vector3 *thisC = getLimitCones()[i]->getControlPoint().copy();
			Vector3 *nextC = getLimitCones()[i + 1]->getControlPoint().copy();
			Rot *thisToNext = new Rot(thisC, nextC);
			Rot *halfThisToNext = new Rot(thisToNext->getAxis(), thisToNext->getAngle() / 2);

			Vector3 *halfAngle = halfThisToNext->applyToCopy(thisC);
			halfAngle->normalize();
			halfAngle->mult(thisToNext->getAngle());
			directions.push_back(halfAngle);

			delete halfThisToNext;
			delete thisToNext;
		}
	}

	Vector3 *newY = new SGVec_3d();
	for (auto dv : directions) {
		newY->add(dv);
	}

	newY->div(directions.size());
	if (newY->mag() != 0 && !std::isnan(newY->y)) {
		newY->normalize();
	} else {
		newY = new SGVec_3d(0, 1, 0);
	}

	SGVec_3d tempVar(0, 0, 0);
	sgRayd *newYRay = new sgRayd(&tempVar, newY);

	Rot *oldYtoNewY = new Rot(limitingAxes_Conflict->y_().heading(), originalLimitingAxes->getGlobalOf(newYRay).heading());
	limitingAxes_Conflict->rotateBy(oldYtoNewY);

	for (auto lc : getLimitCones()) {
		originalLimitingAxes->setToGlobalOf(lc->controlPoint, lc->controlPoint);
		limitingAxes_Conflict->setToLocalOf(lc->controlPoint, lc->controlPoint);
		lc->controlPoint->normalize();
	}

	this->updateTangentRadii();}

void IKKusudama::updateTangentRadii() {
	for (int i = 0; i < limitCones.size(); i++) {
		AbstractLimitCone *next = i < limitCones.size() - 1 ? limitCones[i + 1] : nullptr;
		limitCones[i]->updateTangentHandles(next);
	}
}

void IKKusudama::snapToLimits() {
	// System.out.println("snapping to limits");
	if (orientationallyConstrained) {
		setAxesToOrientationSnap(attachedTo()->localAxes(), limitingAxes_Conflict, 0);
	}
	if (axiallyConstrained) {
		snapToTwistLimits(attachedTo()->localAxes(), limitingAxes_Conflict);
	}
}

void IKKusudama::setAxesToSnapped(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfAngleDampen) {
	if (limitingAxes != nullptr) {
		if (orientationallyConstrained) {
			setAxesToOrientationSnap(toSet, limitingAxes, cosHalfAngleDampen);
		}
		if (axiallyConstrained) {
			snapToTwistLimits(toSet, limitingAxes);
		}
	}
}

void IKKusudama::setAxesToReturnfulled(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfReturnfullness, double angleReturnfullness) {
	if (limitingAxes != nullptr && painfullness > 0) {
		if (orientationallyConstrained) {
			Vector3 *origin = toSet->origin_();
			Vector3 *inPoint = toSet->y_().p2().copy();
			Vector3 *pathPoint = pointOnPathSequence(inPoint, limitingAxes);
			inPoint->sub(origin);
			pathPoint->sub(origin);
			Rot *toClamp = new Rot(inPoint, pathPoint);
			toClamp->rotation.clampToQuadranceAngle(cosHalfReturnfullness);
			toSet->rotateBy(toClamp);
		}
		if (axiallyConstrained) {
			double angleToTwistMid = angleToTwistCenter(toSet, limitingAxes);
			double clampedAngle = MathUtils::clamp(angleToTwistMid, -angleReturnfullness, angleReturnfullness);
			toSet->rotateAboutY(clampedAngle, false);
		}
	}
}

void IKKusudama::setPainfullness(double amt) {
	painfullness = amt;
	if (attachedTo() != nullptr && attachedTo()->parentArmature != nullptr) {
		SegmentedArmature *s = attachedTo()->parentArmature.boneSegmentMap->get(this->attachedTo());
		if (s != nullptr) {
			WorkingBone *wb = s->simulatedBones->get(this->attachedTo());
			if (wb != nullptr) {
				wb->updateCosDampening();
			}
		}
	}
}

double IKKusudama::getPainfullness() {
	return painfullness;
}

void IKKusudama::setAxesToSoftOrientationSnap(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfAngleDampen) {
	Vector<double> inBounds = { 1 };
	/**
	 * Basic idea:
	 * We treat our hard and soft boundaries as if they were two seperate kusudamas.
	 * First we check if we have exceeded our soft boundaries, if so,
	 * we find the closest point on the soft boundary and the closest point on the same segment
	 * of the hard boundary.
	 * let d be our orientation between these two points, represented as a ratio with 0 being right on the soft boundary
	 * and 1 being right on the hard boundary.
	 *
	 * On every kusudama call, we store the previous value of d.
	 * If the new d is  greater than the old d, our result is the weighted average of these
	 * (with the weight determining the resistance of the boundary). This result is stored for reference by future calls.
	 * If the new d is less than the old d, we return the input orientation, and set the new d to this lower value for reference by future calls.
	 *
	 *
	 * Because we can expect rotations to be fairly small, we use nlerp instead of slerp for efficiency when averaging.
	 */
	limitingAxes->updateGlobal();
	boneRay->p1().set(limitingAxes->origin_());
	boneRay->p2().set(toSet->y_().p2());
	Vector3 *bonetip = limitingAxes->getLocalOf(toSet->y_().p2());
	Vector3 *inCushionLimits = this->pointInLimits(bonetip, inBounds, AbstractLimitCone::CUSHION);

	if (inBounds[0] == -1 && inCushionLimits != nullptr) {
		constrainedRay->p1().set(boneRay->p1());
		constrainedRay->p2().set(limitingAxes->getGlobalOf(inCushionLimits));
		Rot *rectifiedRot = new Rot(boneRay->heading(), constrainedRay->heading());
		toSet->rotateBy(rectifiedRot);
		toSet->updateGlobal();
			}
}

void IKKusudama::setAxesToOrientationSnap(AbstractAxes *toSet, AbstractAxes *limitingAxes, double cosHalfAngleDampen) {
	Vector<double> inBounds = { 1 };
	limitingAxes->updateGlobal();
	boneRay->p1().set(limitingAxes->origin_());
	boneRay->p2().set(toSet->y_().p2());
	Vector3 *bonetip = limitingAxes->getLocalOf(toSet->y_().p2());
	Vector3 *inLimits = this->pointInLimits(bonetip, inBounds);

	if (inBounds[0] == -1 && inLimits != nullptr) {
		constrainedRay->p1().set(boneRay->p1());
		constrainedRay->p2().set(limitingAxes->getGlobalOf(inLimits));
		Rot *rectifiedRot = new Rot(boneRay->heading(), constrainedRay->heading());
		toSet->rotateBy(rectifiedRot);
		toSet->updateGlobal();}
}

bool IKKusudama::isInOrientationLimits(AbstractAxes *globalAxes, AbstractAxes *limitingAxes) {
	Vector<double> inBounds = { 1 };
	Vector3 *localizedPoint = limitingAxes->getLocalOf(globalAxes->y_().p2()).copy().normalize();
	if (limitCones.size() == 1) {
		return limitCones[0]->determineIfInBounds(nullptr, localizedPoint);
	}
	for (int i = 0; i < limitCones.size() - 1; i++) {
		if (limitCones[i]->determineIfInBounds(limitCones[i + 1], localizedPoint)) {
			return true;
		}
	}
	return false;
}

void IKKusudama::setAxialLimits(double minAngle, double inRange) {
	minAxialAngle_Conflict = minAngle;
	range = toTau(inRange);
	constraintUpdateNotification();
}

double IKKusudama::snapToTwistLimits(AbstractAxes *toSet, AbstractAxes *limitingAxes) {
	if (!axiallyConstrained) {
		return 0;
	}
	Rot *invRot = limitingAxes->getGlobalMBasis().getInverseRotation();
	Rot *alignRot = invRot->applyTo(toSet->getGlobalMBasis().rotation);
	SGVec_3d tempVar(0, 1, 0);
	Vector<Rot *> decomposition = alignRot->getSwingTwist(&tempVar);
	double angleDelta2 = decomposition[1]->getAngle() * decomposition[1]->getAxis().y * -1;
	angleDelta2 = toTau(angleDelta2);
	double fromMinToAngleDelta = toTau(signedAngleDifference(angleDelta2, TAU - this->minAxialAngle()));

	if (fromMinToAngleDelta < TAU - range) {
		double distToMin = std::abs(signedAngleDifference(angleDelta2, TAU - this->minAxialAngle()));
		double distToMax = std::abs(signedAngleDifference(angleDelta2, TAU - (this->minAxialAngle() + range)));
		double turnDiff = 1;
		turnDiff *= limitingAxes->getGlobalChirality();
		if (distToMin < distToMax) {
			turnDiff = turnDiff * (fromMinToAngleDelta);
			toSet->rotateAboutY(turnDiff, true);
		} else {
			turnDiff = turnDiff * (range - (TAU - fromMinToAngleDelta));
			toSet->rotateAboutY(turnDiff, true);
		}
		return turnDiff < 0 ? turnDiff * -1 : turnDiff;
	} else {
		return 0;
	}
	// return 0;
}

double IKKusudama::angleToTwistCenter(AbstractAxes *toSet, AbstractAxes *limitingAxes) {
	if (!axiallyConstrained) {
		return 0;
	}

	Rot *alignRot = limitingAxes->getGlobalMBasis().getInverseRotation().applyTo(toSet->getGlobalMBasis().rotation);
	SGVec_3d tempVar(0, 1, 0);
	Vector<Rot *> decomposition = alignRot->getSwingTwist(&tempVar);
	double angleDelta2 = decomposition[1]->getAngle() * decomposition[1]->getAxis().y * -1;
	angleDelta2 = toTau(angleDelta2);

	double distToMid = signedAngleDifference(angleDelta2, TAU - (this->minAxialAngle() + (range / 2)));
	return distToMid;
}

bool IKKusudama::inTwistLimits(AbstractAxes *boneAxes, AbstractAxes *limitingAxes) {
	Rot *invRot = limitingAxes->getGlobalMBasis().getInverseRotation();
	Rot *alignRot = invRot->applyTo(boneAxes->getGlobalMBasis().rotation);
	SGVec_3d tempVar(0, 1, 0);
	Vector<Rot *> decomposition = alignRot->getSwingTwist(&tempVar);
	double angleDelta2 = decomposition[1]->getAngle() * decomposition[1]->getAxis().y * -1;
	angleDelta2 = toTau(angleDelta2);
	double fromMinToAngleDelta = toTau(signedAngleDifference(angleDelta2, TAU - this->minAxialAngle()));

	if (fromMinToAngleDelta < TAU - range) {
		double distToMin = std::abs(signedAngleDifference(angleDelta2, TAU - this->minAxialAngle()));
		double distToMax = std::abs(signedAngleDifference(angleDelta2, TAU - (this->minAxialAngle() + range)));
		double turnDiff = 1;
		turnDiff *= limitingAxes->getGlobalChirality();
		if (distToMin < distToMax) {
			return false;
		} else {
			return false;
		}
	}
	return true;
}

double IKKusudama::signedAngleDifference(double minAngle, double __super) {
	double d = std::abs(minAngle - __super) % TAU;
	double r = d > PI ? TAU - d : d;

	double sign = (minAngle - __super >= 0 && minAngle - __super <= PI) || (minAngle - __super <= -PI && minAngle - __super >= -TAU) ? 1.0f : -1.0f;
	r *= sign;
	return r;
}

AbstractBone *IKKusudama::attachedTo() {
	return this->attachedTo_Conflict;
}

void IKKusudama::addLimitCone(SGVec_3d *newPoint, double radius, AbstractLimitCone *previous, AbstractLimitCone *next) {
	int insertAt = 0;

	if (next == nullptr || limitCones.empty()) {
		addLimitConeAtIndex(-1, newPoint, radius);
	} else if (previous != nullptr) {
		insertAt = VectorHelper::indexOf(limitCones, previous) + 1;
	} else {
		insertAt = static_cast<int>(MathUtils::max(0, VectorHelper::indexOf(limitCones, next)));
	}
	addLimitConeAtIndex(insertAt, newPoint, radius);
}

void IKKusudama::removeLimitCone(AbstractLimitCone *limitCone) {
	this->limitCones.remove(limitCone);
	this->updateTangentRadii();
	this->updateRotationalFreedom();
}

void IKKusudama::addLimitConeAtIndex(int insertAt, SGVec_3d *newPoint, double radius) {
	AbstractLimitCone *newCone = createLimitConeForIndex(insertAt, newPoint, radius);
	if (insertAt == -1) {
		limitCones.push_back(newCone);
	} else {
		limitCones.push_back(insertAt, newCone);
	}
	this->updateTangentRadii();
	this->updateRotationalFreedom();
}

double IKKusudama::toTau(double angle) {
	double result = angle;
	if (angle < 0) {
		result = (2 * M_PI) + angle;
	}
	result = result % (M_PI * 2);
	return result;
}

double IKKusudama::mod(double x, double y) {
	if (y != 0 && x != 0) {
		double result = x % y;
		if (result < 0) {
			result += y;
		}
		return result;
	}
	return 0;
}

double IKKusudama::minAxialAngle() {
	return minAxialAngle_Conflict;
}

double IKKusudama::maxAxialAngle() {
	return range;
}

double IKKusudama::absoluteMaxAxialAngle() {
	return signedAngleDifference(range + minAxialAngle_Conflict, M_PI * 2);
}

bool IKKusudama::isAxiallyConstrained() {
	return axiallyConstrained;
}

bool IKKusudama::isOrientationallyConstrained() {
	return orientationallyConstrained;
}

void IKKusudama::disableOrientationalLimits() {
	this->orientationallyConstrained = false;
}

void IKKusudama::enableOrientationalLimits() {
	this->orientationallyConstrained = true;
}

void IKKusudama::toggleOrientationalLimits() {
	this->orientationallyConstrained = !this->orientationallyConstrained;
}

void IKKusudama::disableAxialLimits() {
	this->axiallyConstrained = false;
}

void IKKusudama::enableAxialLimits() {
	this->axiallyConstrained = true;
}

void IKKusudama::toggleAxialLimits() {
	axiallyConstrained = !axiallyConstrained;
}

bool IKKusudama::isEnabled() {
	return axiallyConstrained || orientationallyConstrained;
}

void IKKusudama::disable() {
	this->axiallyConstrained = false;
	this->orientationallyConstrained = false;
}

void IKKusudama::enable() {
	this->axiallyConstrained = true;
	this->orientationallyConstrained = true;
}

double IKKusudama::getRotationalFreedom() {
	// computation cached from updateRotationalFreedom
	// feel free to override that method if you want your own more correct result.
	// please contribute back a better solution if you write one.
	return rotationalFreedom;
}

void IKKusudama::updateRotationalFreedom() {
	double axialConstrainedHyperArea = isAxiallyConstrained() ? (range / TAU) : 1;
	// quick and dirty solution (should revisit);
	double totalLimitConeSurfaceAreaRatio = 0;
	for (auto l : limitCones) {
		totalLimitConeSurfaceAreaRatio += (l->getRadius() * 2) / TAU;
	}
	rotationalFreedom = axialConstrainedHyperArea * (isOrientationallyConstrained() ? std::min(totalLimitConeSurfaceAreaRatio, 1) : 1);
}

void IKKusudama::attachTo(AbstractBone *forBone) {
	this->attachedTo_Conflict = forBone;
	if (this->limitingAxes_Conflict == nullptr) {
		this->limitingAxes_Conflict = forBone->getMajorRotationAxes();
	} else {
		forBone->setFrameofRotation(this->limitingAxes_Conflict);
		this->limitingAxes_Conflict = forBone->getMajorRotationAxes();
	}
}

void IKKusudama::setStrength(double newStrength) {
	this->strength = std::max(0, std::min(1, newStrength));
}

double IKKusudama::getStrength() {
	return this->strength;
}

Vector < ? extends AbstractLimitCone > IKKusudama::getLimitCones() {
	return this->limitCones;
}
