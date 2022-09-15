/*************************************************************************/
/*  limit_cone.h	                                                     */
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

#ifndef LIMIT_CONE_H
#define LIMIT_CONE_H

#include "core/io/resource.h"
#include "core/math/vector3.h"
#include "core/object/ref_counted.h"

#include "ik_bone_segment.h"
#include "kusudama.h"
#include "ray_3d.h"

class IKKusudama;
class LimitCone : public Resource {
	GDCLASS(LimitCone, Resource);

public:
	Vector3 control_point;
	Vector3 radial_point;

	// radius stored as  cosine to save on the acos call necessary for angleBetween.
private:
	double radius_cosine = 0;
	double radius = 0;
	double cushion_radius = 0;
	double cushion_cosine = 0;
	double current_cushion = 1;

public:
	Ref<IKKusudama> parent_kusudama;

	Vector3 tangent_circle_center_next_1;
	Vector3 tangent_circle_center_next_2;
	double tangent_circle_radius_next = 0;
	double tangent_circle_radius_next_cos = 0;

	Vector3 cushion_tangent_circle_center_next_1;
	Vector3 cushion_tangent_circle_center_next_2;
	Vector3 cushion_tangent_circle_center_previous_1;
	Vector3 cushion_tangent_circle_center_previous_2;
	double cushion_tangent_circle_radius_next = 0;
	double cushion_tangent_circle_radius_next_cos = 0;

	static const int BOUNDARY = 0;
	static const int CUSHION = 1;

	// Any softness of 0.0f means completely hard.
	// any softness higher than 0.0f means that
	// as the softness value is increased the more penalty for moving
	// further from the center of the channel.
	double softness = 0;

	/**
	 * A triangle where the [1] is the tangent_circle_next_n, and [0] and [2]
	 * are the points at which the tangent circle intersects this LimitCone and the
	 * next LimitCone.
	 */
	Vector<Vector3> first_triangle_next = { Vector3(), Vector3(), Vector3() };
	Vector<Vector3> second_triangle_next = { Vector3(), Vector3(), Vector3() };

	virtual ~LimitCone() {
	}

	LimitCone();

	LimitCone(Vector3 &direction, double rad, Ref<IKKusudama> attached_to);

	/**
	 *
	 * @param direction
	 * @param rad
	 * @param cushion range 0-1, how far toward the boundary to begin slowing down the rotation if soft constraints are enabled.
	 * Value of 1 creates a hard boundary. Value of 0 means it will always be the case that the closer a joint in the allowable region
	 * is to the boundary, the more any further rotation in the direction of that boundary will be avoided.
	 * @param attached_to
	 */
	LimitCone(Vector3 direction, double rad, double cushion, Ref<IKKusudama> attached_to);

	static Vector3 get_orthogonal(Vector3 p_in);

	/**
	 *
	 * @param next
	 * @param input
	 * @param collision_point will be set to the rectified (if necessary) position of the input after accounting for collisions
	 * @return
	 */
	bool in_bounds_from_this_to_next(Ref<LimitCone> next, Vector3 input, Vector3 collision_point) const;

	/**
	 *
	 * @param next
	 * @param input
	 * @return null if the input point is already in bounds, or the point's rectified position
	 * if the point was out of bounds.
	 */
	Vector3 get_closest_collision(Ref<LimitCone> next, Vector3 input) const;

	Vector3 get_closest_path_point(Ref<LimitCone> next, Vector3 input) const;

	/**
	 * Determines if a ray emanating from the origin to given point in local space
	 * lies within the path from this cone to the next cone. This function relies on
	 * an optimization trick for a performance boost, but the trick ruins everything
	 * if the input isn't normalized. So it is ABSOLUTELY VITAL
	 * that @param input have unit length in order for this function to work correctly.
	 * @param next
	 * @param input
	 * @return
	 */
	bool determine_if_in_bounds(Ref<LimitCone> next, Vector3 input) const;

	Vector3 get_on_path_sequence(Ref<LimitCone> next, Vector3 input) const;

	/**
	 * returns null if no rectification is required.
	 * @param next
	 * @param input
	 * @param in_bounds
	 * @return
	 */
	Vector3 closest_point_on_closest_cone(Ref<LimitCone> next, Vector3 input, Vector<double> &in_bounds) const;

	virtual void update_tangent_handles(Ref<LimitCone> next);

	/**
	 * returns null if no rectification is required.
	 * @param input
	 * @param in_bounds
	 * @return
	 */
	Vector3 closest_to_cone(Vector3 input, Vector<double> &in_bounds) const;

	/**
	 *
	 * @param next
	 * @param input
	 * @return null if inapplicable for rectification. the original point if in bounds, or the point rectified to the closest boundary on the path sequence
	 * between two cones if the point is out of bounds and applicable for rectification.
	 */
	Vector3 get_on_great_tangent_triangle(Ref<LimitCone> next, Vector3 input) const;

private:
	Vector3 closest_cone(Ref<LimitCone> next, Vector3 input) const;

	void update_tangent_and_cushion_handles(Ref<LimitCone> p_next, int p_mode);

	void set_tangent_circle_center_next_1(Vector3 point, int mode);
	void set_tangent_circle_center_next_2(Vector3 point, int mode);

	void set_tangent_circle_radius_next(double rad, int mode);

public:
	virtual Vector3 get_tangent_circle_center_next_1(int mode);

	virtual double get_tangent_circle_radius_next(int mode);

	virtual double get_tangent_circle_radius_next_cos(int mode);

	virtual Vector3 get_tangent_circle_center_next_2(int mode);

protected:
	virtual double _get_radius(int mode);

	virtual double _get_radius_cosine(int mode);

private:
	void compute_triangles(Ref<LimitCone> p_next);

public:
	static Quaternion quaternion_set_axis_angle(Vector3 axis, real_t angle) {
		real_t norm = axis.length();
		if (norm == 0) {
			ERR_PRINT_ONCE("Axis doesn't have a direction.");
			return Quaternion();
		}

		real_t halfAngle = -0.5 * angle;
		real_t coeff = sin(halfAngle) / norm;

		real_t x = coeff * axis.x;
		real_t y = coeff * axis.y;
		real_t z = coeff * axis.z;
		real_t w = cos(halfAngle);
		return Quaternion(x * -1, y * -1, z * -1, w);
	}

	virtual Vector3 get_control_point() const;
	virtual void set_control_point(Vector3 p_control_point);
	virtual double get_radius() const;
	virtual double get_radius_cosine() const;
	virtual void set_radius(double radius);
	virtual double get_cushion_radius();
	virtual double get_cushion_cosine();
	/**
	 * @param p_cushion range 0-1, how far toward the boundary to begin slowing down the rotation if soft constraints are enabled.
	 * Value of 1 creates a hard boundary. Value of 0 means it will always be the case that the closer a joint in the allowable region
	 * is to the boundary, the more any further rotation in the direction of that boundary will be avoided.
	 */
	virtual void set_cushion_boundary(double p_cushion);

	virtual Ref<IKKusudama> get_parent_kusudama();
};
#endif
