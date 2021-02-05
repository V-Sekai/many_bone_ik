/*************************************************************************/
/*  ik_constraint_kusudama.h                                             */
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

#ifndef GODOT_KUSUDAMA_CONSTRAINT_H
#define GODOT_KUSUDAMA_CONSTRAINT_H

#include "core/object/object.h"

#include "core/object/class_db.h"
#include "core/os/memory.h"
#include "direction_constraint.h"
#include "ray.h"
#include "skeleton_modification_3d_ewbik.h"
#include "segmented_skeleton_3d.h"
#include "twist_constraint.h"

class DirectionConstraint;
class EWBIKSegmentedSkeleton3D;

// Kusudama is a papercraft ball with a bunch of cones sticking out of it.
class KusudamaConstraint : public Resource {
	GDCLASS(KusudamaConstraint, Resource);

private:
	/**
     * An array containing all of the Kusudama's directional limits. The kusudama is built up
     * with the expectation that any directional limit in the array is connected to the directional limit at the previous element in the array,
     * and the directional limit at the next element in the array.
     */
	Transform constraint_axes;
	Vector<Ref<DirectionConstraint>> multi_direction;
	int32_t direction_count = 0;
	bool orientation_constrained = false;
	bool axial_constrained = false;
	bool enabled = false;
	Ref<EWBIKSegmentedSkeleton3D> attached_to;
	Ray bone_ray;
	Ray constrained_ray;

	Ref<TwistConstraint> twist = memnew(TwistConstraint);

	float pain = 0.0f;
	float rotational_freedom = 1.0f;

	float get_rotational_freedom() const;

	/**
     * @return a measure of the rotational freedom afforded by this constraint.
     * with 0 meaning no rotational freedom (the bone is essentially stationary in relation to its parent)
     * and 1 meaning full rotational freedom (the bone is completely unconstrained).
     *
     * This should be computed as ratio between orientations a bone can be in and orientations
     * a bone cannot be in as defined by its representation as a point on the surface of a hypersphere.
     */
	virtual void update_rotational_freedom();

private:
	static const int32_t x_axis = 0;
	static const int32_t y_axis = 1;
	static const int32_t z_axis = 2;
protected:
	static void _bind_methods();
public:
	Transform get_constraint_axes() const;
	void set_constraint_axes(Transform p_axes);

	int32_t get_direction_count() const;
	void set_direction_count(int32_t p_count);
	KusudamaConstraint();
	~KusudamaConstraint();

	Ref<DirectionConstraint> get_direction(int32_t p_index) const;

	void set_direction(int32_t p_index, Ref<DirectionConstraint> p_constraint);

	void remove_direction(int32_t p_index);

	KusudamaConstraint(Ref<EWBIKSegmentedSkeleton3D> p_for_bone);

	virtual Ref<TwistConstraint> get_twist_constraint() const;

	virtual void set_twist_constraint(Ref<TwistConstraint> p_twist_constraint);

	virtual void snap_to_limits();

	virtual void disable();

	virtual void enable();

	virtual bool is_enabled() const;

	//returns true if the ray from the constraint origin to the globalPoint is within the constraint's limits
	//false otherwise.
	virtual bool is_in_limits_(const Vector3 p_global_point) const;
	virtual float get_pain();

	/**
     * A value between (ideally between 0 and 1) dictating
     * how much the bone to which this kusudama belongs
     * prefers to be away from the edges of the kusudama
     * if it can. This is useful for avoiding unnatural poses,
     * as the kusudama will push bones back into their more
     * "comfortable" regions. Leave this value at its default of
     * 0 unless you empirical observations show you need it.
     * Setting this value to anything higher than 0.4 is probably overkill
     * in most situations.
     *
     * @param p_amount
     */
	virtual void set_pain(float p_amount);

	virtual float to_tau(float p_angle) const;

	virtual float from_tau(float p_tau) const;

	/**
	 * Given a point (in global coordinates), checks to see if a ray can be extended from the Kusudama's
	 * origin to that point, such that the ray in the Kusudama's reference frame is within the range allowed by the Kusudama's
	 * coneLimits.
	 * If such a ray exists, the original point is returned (the point is within the limits).
	 * If it cannot exist, the tip of the ray within the kusudama's limits that would require the least rotation
	 * to arrive at the input point is returned.
	 * @param inPoint the point to test.
	 * @param returns a number from -1 to 1 representing the point's distance from the boundary, 0 means the point is right on
	 * the boundary, 1 means the point is within the boundary and on the path furthest from the boundary. any negative number means
	 * the point is outside of the boundary, but does not signify anything about how far from the boundary the point is.
	 * @return the original point, if it's in limits, or the closest point which is in limits.
	 */
	virtual Vector3 point_in_limits(Vector3 inPoint, Vector<real_t> inBounds, Transform limitingAxes);

	Vector3 point_on_path_sequence(Transform p_global_xform, Vector3 p_in_point, Transform p_limiting_axes);

	virtual float signed_angle_difference(float p_min_angle, float p_base);

	virtual real_t angle_to_twist_center(Transform p_global_xform, Transform p_to_set, Transform p_limiting_axes);

	virtual void set_axes_to_returnful(Transform p_global_xform, Transform p_to_set, Transform p_limiting_axes,
			real_t p_cos_half_angle_dampen, real_t p_angle_dampen); 
	/**
     *
     * @param p_to_set
     * @param p_limiting_axes
     * @return radians of twist required to snap bone into twist limits (0 if bone is already in twist limits)
     */
	virtual float snap_to_twist_limits(Transform p_to_set, Transform p_limiting_axes);

	/**
     * Presumes the input axes are the bone's localAxes, and rotates
     * them to satisfy the snap limits.
     *
     * @param p_to_set
     */
	virtual void set_axes_to_orientation_snap(Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen);
	;

	virtual void set_axes_to_snapped(Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen);
	;

	virtual void constraint_update_notification();

	/**
     * This function should be called after you've set all of the directional limits
     * for this Kusudama. It will orient the axes relative to which constrained rotations are computed
     * so as to minimize the potential for undesirable twist rotations due to antipodal singularities.
     *
     * In general, auto-optimization attempts to point the y-component of the constraint
     * axes in the direction that places it within an orientation allowed by the constraint,
     * and roughly as far as possible from any orientations not allowed by the constraint.
     */
	virtual void optimize_limiting_axes();

	virtual void update_tangent_radii();

	virtual Ref<DirectionConstraint> create_direction_limit_for_index(int p_insert_at, Vector3 p_new_point, float p_radius);

	/**
     * Adds a direction limit to the Kusudama. Directional limits are reach cones which can be arranged sequentially. The Kusudama will infer
     * a smooth path leading from one direction limit to the next.
     *
     * Using a single direction limit is functionally equivalent to a classic reach cone constraint.
     *
     * @param p_insert_at the intended index for this directional limits in the sequence of direction limits from which the Kusudama will infer a path.
     * @param p_new_point where on the Kusudama to add the directional limit (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
     * @param p_radius the radius of the directional limit
     */
	void add_direction_constraint_at_index(int p_insert_at, Vector3 p_new_point, float p_radius);

	/**
     * Kusudama constraints decompose the bone orientation into a swing component, and a twist component.
     * The "Swing" component is the final direction of the bone. The "Twist" component represents how much
     * the bone is rotated about its own final direction. Where directional limits allow you to constrain the "Swing"
     * component, this method lets you constrain the "twist" component.
     *
     * @param p_min_angle some angle in radians about the major rotation frame's y-axis to serve as the first angle within the range that the bone is allowed to twist.
     * @param p_in_range some angle in radians added to the minAngle. if the bone's local Z goes p_max_angle radians beyond the p_min_angle, it is considered past the limit.
     * This value is always interpreted as being in the positive direction. For example, if this value is -PI/2, the entire range from p_min_angle to p_min_angle + 3PI/4 is
     * considered valid.
     */
	void set_twist_limits(float p_min_angle, float p_in_range);
};

#endif //GODOT_KUSUDAMA_CONSTRAINT_H
