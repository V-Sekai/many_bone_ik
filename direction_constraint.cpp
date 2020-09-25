/*************************************************************************/
/*  ik_direction_limit.cpp                                               */
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


#include "direction_constraint.h"
#include "kusudama_constraint.h"

void DirectionConstraint::initialize(Vector3 p_location, real_t p_rad, Ref<KusudamaConstraint> p_attached_to) {
	set_control_point(p_location);
	tangent_circle_center_next_1 = get_orthogonal(p_location);
	tangent_circle_center_next_2 = tangent_circle_center_next_1 * -1.0f;

	radius = MAX(CMP_EPSILON, p_rad);
	radius_cosine = Math::cos(radius);
	parent_kusudama = p_attached_to;
}

void DirectionConstraint::update_tangent_handles(Ref<DirectionConstraint> p_next) {
	control_point.normalize();

	real_t radA = get_radius();
	real_t radB = p_next->get_radius();

	Vector3 A = get_control_point().normalized();
	Vector3 B = get_control_point().normalized();

	Vector3 arcNormal = A.cross(B);
	Quat aToARadian = Quat(A, arcNormal);
	Vector3 aToARadianAxis;
	real_t aToARadianAngle;
	Basis(aToARadian).get_axis_angle(aToARadianAxis, aToARadianAngle);
	Quat bToBRadian = Quat(B, arcNormal);
	Vector3 bToBRadianAxis;
	real_t bToBRadianAngle;
	Basis(bToBRadian).get_axis_angle(bToBRadianAxis, bToBRadianAngle);
	aToARadian = Quat(aToARadianAxis, radA);
	bToBRadian = Quat(bToBRadianAxis, radB);

	/**
     * There are an infinite number of circles co-tangent with A and B, every other
     * one of which had a unique radius.
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
     * but their radii remain constant), we want our tangentCircle's diameter to be precisely that distance.
     */

	real_t tRadius = ((Math_PI) - (radA + radB)) / 2.0f;

	/**
     * Once we have the desired radius for our tangent circle, we may find the solution for its
     * centers (usually, there are two).
     */

	real_t minorAppoloniusRadiusA = radA + tRadius;
	Vector3 minorAppoloniusAxisA = A.normalized();
	real_t minorAppoloniusRadiusB = radB + tRadius;
	Vector3 minorAppoloniusAxisB = B.normalized();

	//the point on the radius of this cone + half the arcdistance to the circumference of the next cone along the arc path to the next cone
	Vector3 minorAppoloniusP1A = Quat(arcNormal, minorAppoloniusRadiusA).get_euler() * minorAppoloniusAxisA;
	//the point on the radius of this cone + half the arcdistance to the circumference of the next cone, rotated 90 degrees along the axis of this cone
	Vector3 minorAppoloniusP2A = Quat(minorAppoloniusAxisA, Math_PI / 2.0f).get_euler() * minorAppoloniusP1A;
	//the axis of this cone, scaled to minimize its distance to the previous two points.
	Vector3 minorAppoloniusP3A = minorAppoloniusAxisA * Math::cos(minorAppoloniusRadiusA);

	Vector3 minorAppoloniusP1B = Quat(arcNormal, minorAppoloniusRadiusB).get_euler() * minorAppoloniusAxisB;
	Vector3 minorAppoloniusP2B = Quat(minorAppoloniusAxisB, Math_PI / 2.0f).get_euler() * minorAppoloniusP1B;
	Vector3 minorAppoloniusP3B = minorAppoloniusAxisB * Math::cos(minorAppoloniusRadiusB);

	// ray from scaled center of next cone to half way point between the circumference of this cone and the next cone.
	Ray r1B = Ray(minorAppoloniusP1B, minorAppoloniusP3B);
	r1B.elongate(99.0f);
	Ray r2B = Ray(minorAppoloniusP1B, minorAppoloniusP2B);
	r2B.elongate(99.0f);

	Vector3 intersection1;
	Plane plane = Plane(minorAppoloniusP3A, minorAppoloniusP1A, minorAppoloniusP2A);
	plane.intersects_ray(r1B.position, r1B.normal, &intersection1);
	Vector3 intersection2;
	plane.intersects_ray(r2B.position, r2B.normal, &intersection2);

	Ray intersectionRay = Ray(intersection1, intersection2);
	intersectionRay.elongate(99.0f);

	Vector3 sphereIntersect1;
	Vector3 sphereIntersect2;
	Vector3 sphereCenter;

	intersectionRay.intersects_sphere(sphereCenter, 1.0f, sphereIntersect1, sphereIntersect2);

	tangent_circle_center_next_1 = sphereIntersect1;
	tangent_circle_center_next_2 = sphereIntersect2;
	tangent_circle_radius_next = tRadius;

	p_next->tangent_circle_center_previous_1 = sphereIntersect1;
	p_next->tangent_circle_center_previous_2 = sphereIntersect2;
	p_next->tangent_circle_radius_previous = tRadius;

	tangent_circle_radius_next_cos = Math::cos(tangent_circle_radius_next);
	tangent_circle_radius_previous_cos = Math::cos(tangent_circle_radius_previous);

	if (tangent_circle_center_next_1.is_equal_approx(Vector3())) {
		tangent_circle_center_next_1 = get_orthogonal(control_point).normalized();
	}
	if (tangent_circle_center_next_2.is_equal_approx(Vector3())) {
		tangent_circle_center_next_2 = (tangent_circle_center_next_1 * -1.0f).normalized();
	}
	compute_triangles(p_next);
}

void DirectionConstraint::set_radius(real_t p_radius) {
	radius = p_radius;
	radius_cosine = Math::cos(p_radius);
	if (parent_kusudama.is_null()) {
		return;
	}
	parent_kusudama->constraint_update_notification();
}

Vector3 DirectionConstraint::get_orthogonal(Vector3 p_vec) {
	real_t threshold = p_vec.length() * 0.6f;
	if (threshold > 0) {
		if (Math::absf(p_vec.x) <= threshold) {
			real_t inverse = 1 / Math::sqrt(p_vec.y * p_vec.y + p_vec.z * p_vec.z);
			return Vector3(0, inverse * p_vec.z, -inverse * p_vec.y);
		} else if (Math::absf(p_vec.y) <= threshold) {
			real_t inverse = 1 / Math::sqrt(p_vec.x * p_vec.x + p_vec.z * p_vec.z);
			return Vector3(-inverse * p_vec.z, 0, inverse * p_vec.x);
		}
		real_t inverse = 1.0f / Math::sqrt(p_vec.x * p_vec.x + p_vec.y * p_vec.y);
		return Vector3(inverse * p_vec.y, -inverse * p_vec.x, 0);
	}

	return Vector3();
}

Vector3 DirectionConstraint::get_control_point() const {
	return control_point;
}

real_t DirectionConstraint::get_radius_cosine() const {
	return radius_cosine;
}

real_t DirectionConstraint::get_radius() const {
	return radius;
}

Vector3 DirectionConstraint::get_closest_path_point(Ref<DirectionConstraint> p_next, Vector3 p_input) const {
	Vector3 result = get_on_path_sequence(p_next, p_input);
	if (result.is_equal_approx(Vector3())) {
		result = closest_directional_limit(p_next, p_input);
	}
	return result;
}

Vector3 DirectionConstraint::closest_directional_limit(Ref<DirectionConstraint> p_next, Vector3 p_input) const {
	if (p_input.dot(control_point) > p_input.dot(p_next->control_point))
		return control_point;
	else
		return p_next->control_point;
}

Vector3 DirectionConstraint::get_on_path_sequence(Ref<DirectionConstraint> p_next, Vector3 p_input) const {
	Vector3 c1xc2 = control_point.cross(p_next->control_point);
	real_t c1c2fir = p_input.dot(c1xc2);
	if (c1c2fir < 0.0) {
		Vector3 c1xt1 = control_point.cross(tangent_circle_center_next_1);
		Vector3 t1xc2 = tangent_circle_center_next_1.cross(p_next->control_point);
		if (p_input.dot(c1xt1) > 0 && p_input.dot(t1xc2) > 0) {
			Ray tan1ToInput;
			tan1ToInput.position = tangent_circle_center_next_1;
			tan1ToInput.normal = p_input;
			Vector3 result;
			Plane plane = Plane(control_point, tan1ToInput.normal);
			plane.intersects_ray(p_next->control_point, (control_point - p_next->control_point).normalized(), &result);
			return (result - p_next->control_point).normalized();
		} else {
			return Vector3();
		}
	} else {
		Vector3 t2xc1 = tangent_circle_center_next_2.cross(control_point);
		Vector3 c2xt2 = p_next->control_point.cross(tangent_circle_center_next_2);
		if (p_input.dot(t2xc1) > 0 && p_input.dot(c2xt2) > 0) {
			Ray tan2ToInput;
			tan2ToInput.position = tangent_circle_center_next_2;
			tan2ToInput.normal = p_input;
			Vector3 result;

			Plane plane = Plane(control_point, tan2ToInput.normal);
			plane.intersects_ray(p_next->control_point, (control_point - p_next->control_point).normalized(), &result);
			return (result - p_next->control_point).normalized();
		} else {
			return Vector3();
		}
	}
}

void DirectionConstraint::compute_triangles(Ref<DirectionConstraint> p_next) {
	first_triangle_next.resize(3);
	//TODO Move normalization outside
	first_triangle_next.write[1] = tangent_circle_center_next_1.normalized();
	first_triangle_next.write[0] = get_control_point().normalized();
	first_triangle_next.write[2] = p_next->get_control_point().normalized();

	second_triangle_next.resize(3);
	second_triangle_next.write[1] = tangent_circle_center_next_2.normalized();
	second_triangle_next.write[0] = get_control_point().normalized();
	second_triangle_next.write[2] = get_control_point().normalized();
}

void DirectionConstraint::set_control_point(Vector3 p_control_point) {
	control_point = p_control_point;
	if (parent_kusudama.is_valid()) {
		parent_kusudama->constraint_update_notification();
	}
}

void DirectionConstraint::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &DirectionConstraint::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &DirectionConstraint::get_radius);
	ClassDB::bind_method(D_METHOD("set_control_point", "control_point"), &DirectionConstraint::set_control_point);
	ClassDB::bind_method(D_METHOD("get_control_point"), &DirectionConstraint::get_control_point);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "control_point"), "set_control_point", "get_control_point");
}

Vector3 DirectionConstraint::get_on_great_tangent_triangle(Ref<DirectionConstraint> next, Vector3 input) const {
	Vector3 c1xc2 = get_control_point().cross(next->get_control_point());
	float c1c2fir = input.dot(c1xc2);
	if (c1c2fir < 0.0) {
		Vector3 c1xt1 = get_control_point().cross(tangent_circle_center_next_1);
		Vector3 t1xc2 = tangent_circle_center_next_1.cross(next->get_control_point());
		if (input.dot(c1xt1) > 0 && input.dot(t1xc2) > 0) {
			if (input.dot(tangent_circle_center_next_1) > tangent_circle_radius_next_cos) {
				Vector3 planeNormal = tangent_circle_center_next_1.cross(input);
				Quat rotateAboutBy = Quat(planeNormal, tangent_circle_radius_next);
				Transform xform;
				xform.basis = Basis(rotateAboutBy);
				return xform.xform(tangent_circle_center_next_1);
			} else {
				return input;
			}
		} else {
			return Vector3();
		}
	} else {
		Vector3 t2xc1 = tangent_circle_center_next_2.cross(get_control_point());
		Vector3 c2xt2 = next->get_control_point().cross(tangent_circle_center_next_2);
		if (input.dot(t2xc1) > 0 && input.dot(c2xt2) > 0) {
			if (input.dot(tangent_circle_center_next_2) > tangent_circle_radius_next_cos) {
				Vector3 planeNormal = tangent_circle_center_next_2.cross(input);
				Quat rotateAboutBy = Quat(planeNormal, tangent_circle_radius_next);
				Transform xform;
				xform.basis = Basis(rotateAboutBy);
				return xform.xform(tangent_circle_center_next_2);
			} else {
				return input;
			}
		} else {
			return Vector3();
		}
	}
}

Vector3 DirectionConstraint::closest_to_cone(Vector3 input, Vector<bool> inBounds) const {

	if (input.dot(get_control_point()) > get_radius_cosine()) {
		inBounds.write[0] = true;
		return Vector3();
	} else {
		Vector3 axis = get_control_point().cross(input);
		Quat rotTo = Quat(axis, get_radius());
		Transform xform;
		xform.basis = Basis(rotTo);
		Vector3 result = xform.xform(get_control_point());
		inBounds.write[0] = false;
		return result;
	}
}

Vector3
DirectionConstraint::closest_point_on_closest_cone(Ref<DirectionConstraint> next, Vector3 input, Vector<bool> inBounds) const {
	Vector3 closestToFirst = closest_to_cone(input, inBounds);
	if (inBounds[0]) {
		return closestToFirst;
	}
	Vector3 closestToSecond = next->closest_to_cone(input, inBounds);
	if (inBounds[0]) {
		return closestToSecond;
	}

	float cosToFirst = input.dot(closestToFirst);
	float cosToSecond = input.dot(closestToSecond);

	if (cosToFirst > cosToSecond) {
		return closestToFirst;
	} else {
		return closestToSecond;
	}
}

Vector3 DirectionConstraint::get_closest_collision(Ref<DirectionConstraint> next, Vector3 input) const {
	Vector3 result = get_on_great_tangent_triangle(next, input);
	if (result == Vector3()) {
		Vector<bool> inBounds;
		inBounds.resize(1);
		inBounds.write[0] = false;
		result = closest_point_on_closest_cone(next, input, inBounds);
	}
	return result;
}

bool DirectionConstraint::in_bounds_from_this_to_next(Ref<DirectionConstraint> next, Vector3 input, Vector3 collisionPoint) const {
	bool isInBounds = false;
	Vector3 closestCollision = get_closest_collision(next, input);
	if (closestCollision == Vector3()) {
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
