
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

#include "Kusudama.h"

IKKusudama::IKKusudama() {
}

IKKusudama::IKKusudama(Ref<IKBone3D> forBone) {
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
	IKTransform3D originalLimitingAxes = *limitingAxes_Conflict;
	Vector<Vector3> directions;
	if (getLimitCones().size() == 1) {
		directions.push_back((limitCones[0]->getControlPoint())->copy());
	} else {
		for (int i = 0; i < getLimitCones().size() - 1; i++) {
			Vector3 *thisC = getLimitCones()[i]->getControlPoint().copy();
			Vector3 *nextC = getLimitCones()[i + 1]->getControlPoint().copy();
			Rot *thisToNext = new Rot(thisC, nextC);
			Rot *halfThisToNext = new Rot(thisToNext->getAxis(), thisToNext->getAngle() / 2);

			Vector3 halfAngle = halfThisToNext->applyToCopy(thisC);
			halfAngle.normalize();
			halfAngle *= thisToNext->getAngle();
			directions.push_back(halfAngle);
		}
	}

	Vector3 newY;
	for (Vector3 dv : directions) {
		newY += dv;
	}

	newY /= directions.size();
	if (newY->mag() != 0 && !std::isnan(newY->y)) {
		newY.normalize();
	} else {
		newY = Vector3(0, 1, 0);
	}

	Vector3 tempVar(0, 0, 0);
	Ref<Ray3D> newYRay = memnew(Ray3D(&tempVar, newY));

	Rot *oldYtoNewY = memnew(Rot(limitingAxes_Conflict->y_().heading(), originalLimitingAxes->getGlobalOf(newYRay).heading()));
	limitingAxes_Conflict->rotateBy(oldYtoNewY);

	for (auto lc : getLimitCones()) {
		originalLimitingAxes->setToGlobalOf(lc->controlPoint, lc->controlPoint);
		limitingAxes_Conflict->setToLocalOf(lc->controlPoint, lc->controlPoint);
		lc->controlPoint.normalize();
	}

	this->updateTangentRadii();
}

void IKKusudama::updateTangentRadii() {
	for (int i = 0; i < limitCones.size(); i++) {
		Ref<LimitCone> next = i < limitCones.size() - 1 ? limitCones[i + 1] : nullptr;
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

void IKKusudama::setAxesToSnapped(IKTransform3D *toSet, IKTransform3D *limitingAxes, double cosHalfAngleDampen) {
	if (limitingAxes != nullptr) {
		if (orientationallyConstrained) {
			setAxesToOrientationSnap(toSet, limitingAxes, cosHalfAngleDampen);
		}
		if (axiallyConstrained) {
			snapToTwistLimits(toSet, limitingAxes);
		}
	}
}

void IKKusudama::setAxesToReturnfulled(IKTransform3D *toSet, IKTransform3D *limitingAxes, double cosHalfReturnfullness, double angleReturnfullness) {
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

void IKKusudama::setAxesToSoftOrientationSnap(IKTransform3D *toSet, IKTransform3D *limitingAxes, double cosHalfAngleDampen) {
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
	boneRay->p1(limitingAxes->origin_());
	boneRay->p2(toSet->y_().p2());
	Vector3 *bonetip = limitingAxes->getLocalOf(toSet->y_().p2());
	Vector3 *inCushionLimits = this->pointInLimits(bonetip, inBounds, LimitCone::CUSHION);

	if (inBounds[0] == -1 && inCushionLimits != nullptr) {
		constrainedRay->p1(boneRay->p1());
		constrainedRay->p2(limitingAxes->getGlobalOf(inCushionLimits));
		Rot *rectifiedRot = new Rot(boneRay->heading(), constrainedRay->heading());
		toSet->rotateBy(rectifiedRot);
		toSet->updateGlobal();
	}
}

bool IKKusudama::isInOrientationLimits(IKTransform3D *globalAxes, IKTransform3D *limitingAxes) {
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

double IKKusudama::snapToTwistLimits(IKTransform3D *toSet, IKTransform3D *limitingAxes) {
	if (!axiallyConstrained) {
		return 0;
	}
	Rot *invRot = limitingAxes->getGlobalMBasis().getInverseRotation();
	Rot *alignRot = invRot->applyTo(toSet->getGlobalMBasis().rotation);
	Vector3 tempVar(0, 1, 0);
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
	}
	return 0;
}

double IKKusudama::angleToTwistCenter(IKTransform3D *toSet, IKTransform3D *limitingAxes) {
	if (!axiallyConstrained) {
		return 0;
	}

	Rot *alignRot = limitingAxes->getGlobalMBasis().getInverseRotation().applyTo(toSet->getGlobalMBasis().rotation);
	Vector3 tempVar(0, 1, 0);
	Vector<Rot *> decomposition = alignRot->getSwingTwist(&tempVar);
	double angleDelta2 = decomposition[1]->getAngle() * decomposition[1]->getAxis().y * -1;
	angleDelta2 = toTau(angleDelta2);

	double distToMid = signedAngleDifference(angleDelta2, TAU - (this->minAxialAngle() + (range / 2)));
	return distToMid;
}

bool IKKusudama::inTwistLimits(IKTransform3D *boneAxes, IKTransform3D *limitingAxes) {
	Rot *invRot = limitingAxes->getGlobalMBasis().getInverseRotation();
	Rot *alignRot = invRot->applyTo(boneAxes->getGlobalMBasis().rotation);
	Vector3 tempVar(0, 1, 0);
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

Ref<IKBone3D> IKKusudama::attachedTo() {
	return this->attachedTo_Conflict;
}

void IKKusudama::addLimitCone(Vector3 *newPoint, double radius, Ref<LimitCone> previous, Ref<LimitCone> next) {
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

void IKKusudama::removeLimitCone(Ref<LimitCone> limitCone) {
	this->limitCones.remove(limitCone);
	this->updateTangentRadii();
	this->updateRotationalFreedom();
}

void IKKusudama::addLimitConeAtIndex(int insertAt, Vector3 *newPoint, double radius) {
	Ref<LimitCone> newCone = createLimitConeForIndex(insertAt, newPoint, radius);
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

void IKTransform3D::enableOrientationalLimits() {
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

void IKKusudama::attachTo(Ref<IKBone3D> forBone) {
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

Vector<LimitCone> IKKusudama::getLimitCones() {
	return this->limitCones;
}