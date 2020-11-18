/*************************************************************************/
/*  ik_direction_limi.h                                                  */
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


#ifndef GODOT_ANIMATION_UNIFIED_BEZIERS_IK_DIRECTION_LIMIT_H
#define GODOT_ANIMATION_UNIFIED_BEZIERS_IK_DIRECTION_LIMIT_H

#include "core/object/reference.h"
#include "core/io/resource.h"

#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/list.h"
#include "core/variant/variant.h"
#include "core/object/object.h"
#include "core/object/reference.h"
#include "core/templates/hash_map.h"
#include "core/templates/set.h"
#include "core/os/memory.h"
#include "core/os/rw_lock.h"
#include "core/object/class_db.h"
#include "core/typedefs.h"
#include "core/error/error_macros.h"
#include "kusudama_constraint.h"

class KusudamaConstraint;
class DirectionConstraint : public Resource {
	GDCLASS(DirectionConstraint, Resource);
private:
	/**
     * a triangle where the [1] is the tangentCircleNext_n, and [0] and [2]
     * are the points at which the tangent circle intersects this directional limit and the
     * next directional limit
     */
	Vector<Vector3> first_triangle_next;
	Vector<Vector3> second_triangle_next;

	//radius stored as cosine to save on the acos call necessary for angleBetween.
	float radius = 1.0f;
	float radius_cosine = Math::cos(radius);
	Ref<KusudamaConstraint> parent_kusudama = nullptr;
	Vector3 tangent_circle_center_next_1;
	Vector3 tangent_circle_center_next_2;
	float tangent_circle_radius_next = 0.0f;
	float tangent_circle_radius_next_cos = 0.0f;
	Vector3 tangent_circle_center_previous_1;
	Vector3 tangent_circle_center_previous_2;
	float tangent_circle_radius_previous = 0.0f;
	float tangent_circle_radius_previous_cos = 0.0f;

	void compute_triangles(Ref<DirectionConstraint> p_next);

protected:
	static void _bind_methods();

public:
	~DirectionConstraint() {
	}

	Vector3 get_on_great_tangent_triangle(Ref<DirectionConstraint> next, Vector3 input) const;

	/**
	 * returns Vector3(0, 0, 0) if no rectification is required.
	 * @param input
	 * @param inBounds
	 * @return
	 */
	Vector3 closest_to_cone(Vector3 input, Vector<bool> inBounds) const;

	/**
	 * returns Vector3(0, 0, 0) if no rectification is required.
	 * @param next
	 * @param input
	 * @param inBounds
	 * @return
	 */
	Vector3 closest_point_on_closest_cone(Ref<DirectionConstraint> next, Vector3 input, Vector<bool> inBounds) const;

	/**
	 *
	 * @param next
	 * @param input
	 * @return Vector3(0, 0, 0) if the input point is already in bounds, or the point's rectified position
	 * if the point was out of bounds.
	 */
	Vector3 get_closest_collision(Ref<DirectionConstraint> next, Vector3 input) const;

	/**
	 *
	 * @param next
	 * @param input
	 * @param collisionPoint will be set to the rectified (if necessary) position of the input after accounting for collisions
	 * @return
	 */
	bool in_bounds_from_this_to_next(Ref<DirectionConstraint> next, Vector3 input, Vector3 collisionPoint) const;

	Vector3 control_point;
	Vector3 radial_point;

	Vector3 get_on_path_sequence(Ref<DirectionConstraint> p_next, Vector3 p_input) const;
	Vector3 closest_directional_limit(Ref<DirectionConstraint> p_next, Vector3 p_input) const;
	Vector3 get_closest_path_point(Ref<DirectionConstraint> p_next, Vector3 p_input) const;
	float get_radius() const;
	float get_radius_cosine() const;
	Vector3 get_control_point() const;
	Vector3 get_orthogonal(Vector3 p_vec);
	DirectionConstraint() {}
	void set_control_point(Vector3 p_control_point);
	void update_tangent_handles(Ref<DirectionConstraint> p_next);
	void initialize(Vector3 p_location, float p_rad, Ref<KusudamaConstraint> p_attached_to);
	void set_radius(float p_radius);
};

#endif //GODOT_ANIMATION_UNIFIED_BEZIERS_IK_DIRECTION_LIMIT_H
