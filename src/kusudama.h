
/*************************************************************************/
/*  kusudama.h                                                           */
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
#include "ik_bone_segment.h"
#include "kusudama.h"
#include "limit_cone.h"
#include "math/ik_transform.h"
#include "ray_3d.h"

class IKBone3D;
class LimitCone;
class IKKusudama : public Resource {
	GDCLASS(IKKusudama, Resource);

protected:
	Ref<IKTransform3D> _limiting_axes = Ref<IKTransform3D>(memnew(IKTransform3D()));
	double painfullness = 0;

	/**
	 * An array containing all of the Kusudama's limit_cones. The kusudama is built up
	 * with the expectation that any limitCone in the array is connected to the cone at the previous element in the array,
	 * and the cone at the next element in the array.
	 */
	Vector<Ref<LimitCone>> limit_cones;

	/**
	 * Defined as some Angle in radians about the limiting_axes Y axis, 0 being equivalent to the
	 * limiting_axes Z axis.
	 */
	double _min_axial_angle = Math_PI;
	/**
	 * Defined as some Angle in radians about the limiting_axes Y axis, 0 being equivalent to the
	 * min_axial_angle
	 */
	double range = Math_PI * 3;

	bool orientationally_constrained = false;
	bool axially_constrained = false;

	// for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	// per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.
	real_t strength = 1;

	Ref<IKBone3D> _attached_to;

public:
	static const int BOUNDARY = 0;
	static const int CUSHION = 1;

	virtual ~IKKusudama() {
	}

	IKKusudama();

	IKKusudama(Ref<IKBone3D> for_bone);

	IKKusudama(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen);

	virtual void _update_constraint();

	virtual void update_tangent_radii();

	Ref<Ray3D> bone_ray = Ref<Ray3D>(memnew(Ray3D()));
	Ref<Ray3D> constrained_ray = Ref<Ray3D>(memnew(Ray3D()));

	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param to_set
	 */
	virtual void set_axes_to_soft_orientation_snap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen);

public:
	/**
	 * Presumes the input axes are the bone's localAxes, and rotates
	 * them to satisfy the snap limits.
	 *
	 * @param to_set
	 */
	virtual void set_axes_to_orientation_snap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double p_dampen, double p_cos_half_angle_dampen);

	virtual bool is_in_global_pose_orientation_limits(Ref<IKTransform3D> global_axes, Ref<IKTransform3D> limiting_axes);

	/**
	 * Kusudama constraints decompose the bone orientation into a swing component, and a twist component.
	 * The "Swing" component is the final direction of the bone. The "Twist" component represents how much
	 * the bone is rotated about its own final direction. Where limit cones allow you to constrain the "Swing"
	 * component, this method lets you constrain the "twist" component.
	 *
	 * @param min_angle some angle in radians about the major rotation frame's y-axis to serve as the first angle within the range that the bone is allowed to twist.
	 * @param in_range some angle in radians added to the min_angle. if the bone's local Z goes maxAngle radians beyond the min_angle, it is considered past the limit.
	 * This value is always interpreted as being in the positive direction. For example, if this value is -PI/2, the entire range from min_angle to min_angle + 3PI/4 is
	 * considered valid.
	 */
	virtual void set_axial_limits(double min_angle, double in_range);

	// protected CartesianAxes limitLocalAxes;

	/**
	 *
	 * @param to_set
	 * @param limiting_axes
	 * @return radians of the twist required to snap bone into twist limits (0 if bone is already in twist limits)
	 */
	virtual void set_snap_to_twist_limit(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, float p_dampening, float p_cos_half_dampen);

	virtual double angle_to_twist_center(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes);

	virtual bool in_twist_limits(Ref<IKTransform3D> bone_axes, Ref<IKTransform3D> limiting_axes);

	virtual double signed_angle_difference(double min_angle, double p_super);

	/**
	 * Given a point (in local coordinates), checks to see if a ray can be extended from the Kusudama's
	 * origin to that point, such that the ray in the Kusudama's reference frame is within the range allowed by the Kusudama's
	 * coneLimits.
	 * If such a ray exists, the original point is returned (the point is within the limits).
	 * If it cannot exist, the tip of the ray within the kusudama's limits that would require the least rotation
	 * to arrive at the input point is returned.
	 * @param in_point the point to test.
	 * @param in_bounds should be an array with at least 2 elements. The first element will be set to  a number from -1 to 1 representing the point's distance from the boundary, 0 means the point is right on
	 * the boundary, 1 means the point is within the boundary and on the path furthest from the boundary. any negative number means
	 * the point is outside of the boundary, but does not signify anything about how far from the boundary the point is.
	 * The second element will be given a value corresponding to the limit cone whose bounds were exceeded. If the bounds were exceeded on a segment between two limit cones,
	 * this value will be set to a non-integer value between the two indices of the limitcone comprising the segment whose bounds were exceeded.
	 * @return the original point, if it's in limits, or the closest point which is in limits.
	 */
	Vector3 _local_point_in_limits(Vector3 in_point, Vector<double> &in_bounds, int mode = IKKusudama::CUSHION);

	Vector3 local_point_on_path_sequence(Vector3 in_point, Ref<IKTransform3D> limiting_axes);

	// public double softLimit

	virtual Ref<IKBone3D> attached_to();

	/**
	 * Add a LimitCone to the Kusudama.
	 * @param new_point where on the Kusudama to add the LimitCone (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
	 * @param radius the radius of the limitCone
	 * @param previous the LimitCone adjacent to this one (may be null if LimitCone is not supposed to be between two existing LimitCones)
	 * @param next the other LimitCone adjacent to this one (may be null if LimitCone is not supposed to be between two existing LimitCones)
	 */
	virtual void add_limit_cone(Vector3 new_point, double radius, Ref<LimitCone> previous = Ref<LimitCone>(), Ref<LimitCone> next = Ref<LimitCone>());

	virtual void remove_limit_cone(Ref<LimitCone> limitCone);

	/**
	 * Adds a LimitCone to the Kusudama. LimitCones are reach cones which can be arranged sequentially. The Kusudama will infer
	 * a smooth path leading from one LimitCone to the next.
	 *
	 * Using a single LimitCone is functionally equivalent to a classic reachCone constraint.
	 *
	 * @param insert_at the intended index for this LimitCone in the sequence of LimitCones from which the Kusudama will infer a path. @see IK.IKKusudama.limit_cones limit_cones array.
	 * @param new_point where on the Kusudama to add the LimitCone (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
	 * @param radius the radius of the limitCone
	 */
	void add_limit_cone_at_index(int insert_at, Vector3 new_point, double radius);

	static double to_tau(double angle);

	virtual double mod(double x, double y);

	/**
	 * @return the limiting_axes of this Kusudama (these are it's parentBone's majorRotationAxes)
	 */
	Ref<IKTransform3D> limiting_axes();

	/**
	 *
	 * @return the lower bound on the axial constraint
	 */
	virtual double min_axial_angle();

	virtual double max_axial_angle();

	/**
	 * the upper bound on the axial constraint in absolute terms
	 * @return
	 */
	virtual double absolute_max_axial_angle();

	virtual bool is_axially_constrained();

	virtual bool is_orientationally_constrained();

	virtual void disable_orientational_limits();

	virtual void enable_orientational_limits();

	virtual void toggle_orientational_limits();

	virtual void disable_axial_limits();

	virtual void enable_axial_limits();

	virtual void toggle_axial_limits();

	bool is_enabled();

	void disable();

	void enable();

	double unit_hyper_area = 2 * Math::pow(Math_PI, 2);
	double unit_area = 4 * Math_PI;

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
	double get_rotational_freedom();

	double rotational_freedom = 1;

	virtual void update_rotational_freedom();

protected:
	/**
	 * attaches the Kusudama to the BoneExample. If the
	 * kusudama has its own limiting axes specified,
	 * replaces the bone's major rotation
	 * axes with the Kusudamas limiting axes.
	 *
	 * otherwise, this function will set the kusudama's
	 * limiting axes to the major rotation axes specified by the bone.
	 *
	 * @param for_bone the bone to which to attach this Kusudama.
	 */
public:
	/**for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	 per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.
	 @param strength a value between 0 and 1. Any other value will be clamped to this range.
	 **/
	virtual void set_strength(double newStrength);

	/**for IK solvers. Defines the weight ratio between the unconstrained IK solved orientation and the constrained orientation for this bone
	 per iteration. This should help stabilize solutions somewhat by allowing for soft constraint violations.**/
	virtual double get_strength() const;

	virtual Vector<Ref<LimitCone>> get_limit_cones() const;

	virtual void set_limit_cones(Vector<Ref<LimitCone>> p_cones) {
		limit_cones = p_cones;
	}

public:
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
	static void get_swing_twist(
			Quaternion p_rotation,
			Vector3 p_axis,
			Quaternion &swing,
			Quaternion &twist) {
		Quaternion rotation = p_rotation;
		rotation.x *= -1;
		rotation.y *= -1;
		rotation.z *= -1;
		Vector3 quaternion_axis;
		quaternion_axis.x = rotation.x;
		quaternion_axis.y = rotation.y;
		quaternion_axis.z = rotation.z;
		const float d = quaternion_axis.dot(p_axis);
		twist = Quaternion(p_axis.x * d, p_axis.y * d, p_axis.z * d, rotation.w).normalized();
		if (d < 0) {
			twist *= -1.0f;
		}
		swing = twist;
		swing.x *= -1;
		swing.y *= -1;
		swing.z *= -1;
		swing = twist * p_rotation;
		swing.normalize();
		swing.x *= -1;
		swing.y *= -1;
		swing.z *= -1;
		twist.x *= -1;
		twist.y *= -1;
		twist.z *= -1;
	}
};
#endif
