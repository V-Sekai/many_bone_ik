
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

#ifndef KUSUDAMA_H
#define KUSUDAMA_H

#include "core/io/resource.h"
#include "core/math/quaternion.h"
#include "core/math/vector3.h"
#include "core/object/ref_counted.h"

#include "ik_bone_3d.h"
#include "ik_bone_chain.h"
#include "kusudama.h"
#include "limit_cone.h"
#include "math/ik_transform.h"
#include "ray_3d.h"

class IKBone3D;
class LimitCone;
class IKKusudama : public Resource {
	GDCLASS(IKKusudama, Resource);

protected:
	Ref<IKTransform3D> limitingAxes_Conflict = Ref<IKTransform3D>(memnew(IKTransform3D()));
	double painfullness = 0;

	/**
	 * An array containing all of the Kusudama's limitCones. The kusudama is built up
	 * with the expectation that any limitCone in the array is connected to the cone at the previous element in the array,
	 * and the cone at the next element in the array.
	 */
	Vector<Ref<LimitCone>> limitCones;

	/**
	 * Defined as some Angle in radians about the limiting_axes Y axis, 0 being equivalent to the
	 * limiting_axes Z axis.
	 */
	double minAxialAngle_Conflict = Math_PI;
	/**
	 * Defined as some Angle in radians about the limiting_axes Y axis, 0 being equivalent to the
	 * minAxialAngle
	 */
	double range = Math_PI * 3;

	bool orientationallyConstrained = false;
	bool axiallyConstrained = false;

	// for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	// per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.
	real_t strength = 1;

	Ref<IKBone3D> attachedTo_Conflict;

public:
	static const int BOUNDARY = 0;
	static const int CUSHION = 1;

	virtual ~IKKusudama() {
	}

	IKKusudama();

	IKKusudama(Ref<IKBone3D> forBone);

	IKKusudama(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen);

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

	Ref<Ray3D> boneRay = Ref<Ray3D>(memnew(Ray3D()));
	Ref<Ray3D> constrainedRay = Ref<Ray3D>(memnew(Ray3D()));

	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param to_set
	 */
	virtual void setAxesToSnapped(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen);

	// virtual void setAxesToReturnfulled(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double cosHalfReturnfullness, double angleReturnfullness);

	// /**
	//  * A value between (ideally between 0 and 1) dictating
	//  * how much the bone to which this kusudama belongs
	//  * prefers to be away from the edges of the kusudama
	//  * if it can. This is useful for avoiding unnatural poses,
	//  * as the kusudama will push bones back into their more
	//  * "comfortable" regions. Leave this value at its default of
	//  * 0 unless you empircal observations show you need it.
	//  * Setting this value to anything higher than 0.4 is probably overkill
	//  * in most situations.
	//  *
	//  * @param amt
	//  */
	// virtual void setPainfullness(double amt);

	// /**
	//  * @return A value between (ideally between 0 and 1) dictating
	//  * how much the bone to which this kusudama belongs
	//  * prefers to be away from the edges of the kusudama
	//  * if it can.
	//  */
	// virtual double getPainfullness();

	bool isInLimits_(Vector3 globalPoint);

	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param to_set
	 */
	virtual void setAxesToSoftOrientationSnap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> slimitingAxes, double cos_half_angle_dampen);

private:
	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param to_set
	 */
	virtual void setAxesToOrientationSnap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen);
public:

	virtual bool isInOrientationLimits(Ref<IKTransform3D> globalAxes, Ref<IKTransform3D> limiting_axes);

	/**
	 * Kusudama constraints decompose the bone orientation into a swing component, and a twist component.
	 * The "Swing" component is the final direction of the bone. The "Twist" component represents how much
	 * the bone is rotated about its own final direction. Where limit cones allow you to constrain the "Swing"
	 * component, this method lets you constrain the "twist" component.
	 *
	 * @param minAngle some angle in radians about the major rotation frame's y-axis to serve as the first angle within the range that the bone is allowed to twist.
	 * @param inRange some angle in radians added to the minAngle. if the bone's local Z goes maxAngle radians beyond the minAngle, it is considered past the limit.
	 * This value is always interpreted as being in the positive direction. For example, if this value is -PI/2, the entire range from minAngle to minAngle + 3PI/4 is
	 * considered valid.
	 */
	virtual void setAxialLimits(double minAngle, double inRange);

	// protected CartesianAxes limitLocalAxes;

	/**
	 *
	 * @param to_set
	 * @param limiting_axes
	 * @return radians of the twist required to snap bone into twist limits (0 if bone is already in twist limits)
	 */
	virtual double snapToTwistLimits(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes);

	virtual double angleToTwistCenter(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes);

	virtual bool inTwistLimits(Ref<IKTransform3D> boneAxes, Ref<IKTransform3D> limiting_axes);

	virtual double signedAngleDifference(double minAngle, double p_super);

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
	Vector3 pointInLimits(Vector3 inPoint, Vector<double> &inBounds, int mode = IKKusudama::CUSHION);

	Vector3 pointOnPathSequence(Vector3 inPoint, Ref<IKTransform3D> limiting_axes);

	// public double softLimit

	virtual Ref<IKBone3D> attachedTo();

	/**
	 * Add a LimitCone to the Kusudama.
	 * @param newPoint where on the Kusudama to add the LimitCone (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
	 * @param radius the radius of the limitCone
	 * @param previous the LimitCone adjacent to this one (may be null if LimitCone is not supposed to be between two existing LimitCones)
	 * @param next the other LimitCone adjacent to this one (may be null if LimitCone is not supposed to be between two existing LimitCones)
	 */
	virtual void addLimitCone(Vector3 newPoint, double radius, Ref<LimitCone> previous, Ref<LimitCone> next);

	virtual void removeLimitCone(Ref<LimitCone> limitCone);

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
	void addLimitConeAtIndex(int insertAt, Vector3 newPoint, double radius);

	virtual double toTau(double angle);

	virtual double mod(double x, double y);

	/**
	 * @return the limiting_axes of this Kusudama (these are it's parentBone's majorRotationAxes)
	 */
	Ref<IKTransform3D> limiting_axes() {
		// if(inverted) return inverseLimitingAxes;
		return limitingAxes_Conflict;
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

	bool isEnabled();

	void disable();

	void enable();

	double unitHyperArea = 2 * Math::pow(Math_PI, 2);
	double unitArea = 4 * Math_PI;

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
	double getRotationalFreedom();

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
	/**for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	 per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.
	 @param strength a value between 0 and 1. Any other value will be clamped to this range.
	 **/
	virtual void setStrength(double newStrength);

	/**for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	 per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.**/
	virtual double getStrength() const;

	virtual Vector<Ref<LimitCone>> getLimitCones();

	/**
	 * Get the swing rotation and twist rotation for the specified axis. The twist
	 * rotation represents the rotation around the
	 * specified axis. The swing rotation represents the rotation of the specified
	 * axis itself, which is the rotation around an
	 * axis perpendicular to the specified axis. The swing and twist rotation can be
	 * used to reconstruct the original
	 * quaternion: this = swing * twist
	 *
	 * @param axisX the X component of the normalized axis for which to get the
	 *              swing and twist rotation
	 * @param axisY the Y component of the normalized axis for which to get the
	 *              swing and twist rotation
	 * @param axisZ the Z component of the normalized axis for which to get the
	 *              swing and twist rotation
	 * @return an Array of Quaternion objects. With the first element representing
	 *         the
	 *         swing, and the second representing the twist
	 * @see <a href=
	 *      "http://www.euclideanspace.com/maths/geometry/rotations/for/decomposition">calculation</a>
	 */
public:
	static Vector<Quaternion> getSwingTwist(Quaternion p_quaternion, Vector3 p_axis);

	/**
	 * Build one of the rotations that transform one vector into another one.
	 * <p>
	 * Except for a possible scale factor, if the instance were
	 * applied to the vector u it will produce the vector v. There is an
	 * infinite number of such rotations, this constructor choose the
	 * one with the smallest associated angle (i.e. the one whose axis
	 * is orthogonal to the (u, v) plane). If u and v are colinear, an
	 * arbitrary rotation axis is chosen.
	 * </p>
	 *
	 * @param u origin vector
	 * @param v desired image of u by the rotation
	 * @throws MathUtils.MathArithmeticException if the norm of one of the vectors
	 *                                           is zero
	 */
public:
	static Quaternion build_rotation_from_headings(Vector3 u, Vector3 v);
};
#endif