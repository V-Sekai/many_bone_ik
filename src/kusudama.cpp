/*************************************************************************/
/*  kusudama.cpp                                                         */
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

#include "kusudama.h"
#include "math/ik_transform.h"

IKKusudama::IKKusudama() {
}

IKKusudama::IKKusudama(Ref<IKBone3D> for_bone) {
	this->_attached_to = for_bone;
	this->_limiting_axes->set_global_transform(for_bone->get_global_pose());
	this->_attached_to->add_constraint(Ref<IKKusudama>(this));
	this->enable();
}

void IKKusudama::_update_constraint() {
	update_tangent_radii();
	update_rotational_freedom();
}

void IKKusudama::update_tangent_radii() {
	for (int i = 0; i < limit_cones.size(); i++) {
		Ref<LimitCone> current = limit_cones[i];
		Ref<LimitCone> next;
		if (i < limit_cones.size() - 1) {
			next = limit_cones.write[i + 1];
		}
		limit_cones.write[i]->update_tangent_handles(next);
	}
}

IKKusudama::IKKusudama(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen) {
	Vector<double> in_bounds = { 1 };
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
	 * Because we can expect rotations to be fairly small, we use nlerp instead of slerp for efficiency when averaging.
	 */
}

bool IKKusudama::is_in_global_pose_orientation_limits(Ref<IKTransform3D> p_global_axes, Ref<IKTransform3D> p_limiting_axes) {
	Vector<double> in_bounds = { 1 };
	Vector3 global_y_heading = p_global_axes->get_global_transform().basis.get_column(Vector3::AXIS_Y);
	global_y_heading = global_y_heading + p_global_axes->get_global_transform().origin;
	Vector3 local_point = p_limiting_axes->to_local(global_y_heading);
	Vector3 in_limits = _local_point_in_limits(local_point, in_bounds, IKKusudama::BOUNDARY);
	bool is_rotation = !Math::is_nan(in_limits.x);
	if (in_bounds[0] < 0.0 || !is_rotation) {
		return false;
	}
	return true;
}

void IKKusudama::set_axial_limits(double min_angle, double in_range) {
	_min_axial_angle = min_angle;
	range = to_tau(in_range);
	_update_constraint();
}

void IKKusudama::set_snap_to_twist_limit(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, float p_dampening, float p_cos_half_dampen) {
	Basis inv_rot = limiting_axes->get_global_transform().basis.get_quaternion().inverse();
	Basis align_rot = inv_rot * to_set->get_global_transform().basis;
	Quaternion swing;
	Quaternion twist;
	get_swing_twist(align_rot.get_rotation_quaternion(), Vector3(0, 1, 0), swing, twist);
	double angle_delta_2 = twist.get_angle() * twist.get_axis().y * -1;
	angle_delta_2 = to_tau(angle_delta_2);
	double from_min_to_angle_delta = to_tau(signed_angle_difference(angle_delta_2, Math_TAU - min_axial_angle()));
	if (!(from_min_to_angle_delta < Math_TAU - range)) {
		return;
	}
	double dist_to_min = Math::abs(signed_angle_difference(angle_delta_2, Math_TAU - min_axial_angle()));
	double dist_to_max = Math::abs(signed_angle_difference(angle_delta_2, Math_TAU - (min_axial_angle() + range)));
	double turn_diff = 1;
	turn_diff *= limiting_axes->get_global_chirality();
	Quaternion rot;
	Vector3 axis_y = to_set->get_global_transform().basis.get_column(Vector3::AXIS_Y);
	if (dist_to_min < dist_to_max) {
		turn_diff = turn_diff * (from_min_to_angle_delta);
	} else {
		turn_diff = turn_diff * (range - (Math_TAU - from_min_to_angle_delta));
	}
	rot = Quaternion(axis_y.normalized(), turn_diff).normalized();
	to_set->rotate_local_with_global(rot);
}

double IKKusudama::angle_to_twist_center(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes) {
	if (!axially_constrained) {
		return 0;
	}
	Quaternion align_rot = limiting_axes->get_global_transform().basis.inverse() * to_set->get_global_transform().basis;
	Vector3 temp_var(0, 1, 0);
	Quaternion swing;
	Quaternion twist;
	get_swing_twist(align_rot, temp_var, swing, twist);
	double angle_delta_2 = twist.get_angle() * Basis(twist).get_euler().y * -1;
	angle_delta_2 = to_tau(angle_delta_2);
	double dist_to_mid = signed_angle_difference(angle_delta_2, Math_TAU - (this->min_axial_angle() + (range / 2)));
	return dist_to_mid;
}

bool IKKusudama::in_twist_limits(Ref<IKTransform3D> bone_axes, Ref<IKTransform3D> limiting_axes) {
	Basis inv_rot = limiting_axes->get_global_transform().basis.inverse();
	Basis align_rot = inv_rot * bone_axes->get_global_transform().basis;
	Vector3 temp_var(0, 1, 0);
	Quaternion swing;
	Quaternion twist;
	get_swing_twist(align_rot, temp_var, swing, twist);
	double angle_delta_2 = twist.get_angle() * twist.get_axis().y * -1;
	angle_delta_2 = to_tau(angle_delta_2);
	double from_min_to_angle_delta = to_tau(signed_angle_difference(angle_delta_2, Math_TAU - this->min_axial_angle()));

	if (from_min_to_angle_delta < Math_TAU - range) {
		double dist_to_min = Math::abs(signed_angle_difference(angle_delta_2, Math_TAU - this->min_axial_angle()));
		double dist_to_max = Math::abs(signed_angle_difference(angle_delta_2, Math_TAU - (this->min_axial_angle() + range)));
		if (dist_to_min < dist_to_max) {
			return false;
		} else {
			return false;
		}
	}
	return true;
}

double IKKusudama::signed_angle_difference(double min_angle, double p_super) {
	double d = Math::fmod(Math::abs(min_angle - p_super), Math_TAU);
	double r = d > Math_PI ? Math_TAU - d : d;

	double sign = (min_angle - p_super >= 0 && min_angle - p_super <= Math_PI) || (min_angle - p_super <= -Math_PI && min_angle - p_super >= -Math_TAU) ? 1.0f : -1.0f;
	r *= sign;
	return r;
}

Ref<IKBone3D> IKKusudama::attached_to() {
	return this->_attached_to;
}

void IKKusudama::add_limit_cone(Vector3 new_cone_local_point, double radius, Ref<LimitCone> previous, Ref<LimitCone> next) {
	Vector3 localized_point = new_cone_local_point;
	if (_attached_to.is_valid() && _attached_to->get_parent().is_valid()) {
		Vector3 globalized_point = _attached_to->get_parent()->get_ik_transform()->get_global_transform().xform(new_cone_local_point);
		Vector3 offset = _attached_to->get_parent()->get_ik_transform()->get_global_transform().origin - _limiting_axes->get_global_transform().origin;
		globalized_point += offset;
		localized_point = _limiting_axes->to_local(globalized_point);
	}

	int insert_at = 0;
	if (next.is_null() || limit_cones.is_empty()) {
		add_limit_cone_at_index(insert_at, localized_point, radius);
		return;
	}
	if (previous.is_valid()) {
		insert_at = limit_cones.find(previous) + 1;
	} else {
		insert_at = MAX(0, limit_cones.find(next));
	}
	add_limit_cone_at_index(insert_at, localized_point, radius);
}

void IKKusudama::remove_limit_cone(Ref<LimitCone> limitCone) {
	this->limit_cones.erase(limitCone);
}

void IKKusudama::add_limit_cone_at_index(int insert_at, Vector3 new_cone_local_point, double radius) {
	Ref<LimitCone> newCone = memnew(LimitCone(new_cone_local_point, radius, Ref<IKKusudama>(this)));
	limit_cones.insert(insert_at, newCone);
}

double IKKusudama::to_tau(double angle) {
	double result = angle;
	if (angle < 0) {
		result = (2 * Math_PI) + angle;
	}
	result = Math::fmod(result, (Math_PI * 2.0f));
	return result;
}

double IKKusudama::mod(double x, double y) {
	if (!Math::is_zero_approx(y) && !Math::is_zero_approx(x)) {
		double result = Math::fmod(x, y);
		if (result < 0.0f) {
			result += y;
		}
		return result;
	}
	return 0.0f;
}

double IKKusudama::min_axial_angle() {
	return _min_axial_angle;
}

double IKKusudama::max_axial_angle() {
	return range;
}

double IKKusudama::absolute_max_axial_angle() {
	return signed_angle_difference(range + _min_axial_angle, Math_TAU);
}

bool IKKusudama::is_axially_constrained() {
	return axially_constrained;
}

bool IKKusudama::is_orientationally_constrained() {
	return orientationally_constrained;
}

void IKKusudama::disable_orientational_limits() {
	this->orientationally_constrained = false;
}

void IKKusudama::enable_orientational_limits() {
	this->orientationally_constrained = true;
}

void IKKusudama::toggle_orientational_limits() {
	this->orientationally_constrained = !this->orientationally_constrained;
}

void IKKusudama::disable_axial_limits() {
	this->axially_constrained = false;
}

void IKKusudama::enable_axial_limits() {
	this->axially_constrained = true;
}

void IKKusudama::toggle_axial_limits() {
	axially_constrained = !axially_constrained;
}

bool IKKusudama::is_enabled() {
	return axially_constrained || orientationally_constrained;
}

void IKKusudama::disable() {
	this->axially_constrained = false;
	this->orientationally_constrained = false;
}

void IKKusudama::enable() {
	this->axially_constrained = true;
	this->orientationally_constrained = true;
}

double IKKusudama::get_rotational_freedom() {
	// computation cached from update_rotational_freedom
	// feel free to override that method if you want your own more correct result.
	// please contribute back a better solution if you write one.
	return rotational_freedom;
}

void IKKusudama::update_rotational_freedom() {
	double axial_constrained_hyper_area = is_axially_constrained() ? (range / Math_TAU) : 1;
	// quick and dirty solution (should revisit);
	double total_limit_cone_surface_area_ratio = 0;
	for (Ref<LimitCone> l : limit_cones) {
		total_limit_cone_surface_area_ratio += (l->get_radius() * 2) / Math_TAU;
	}
	rotational_freedom = axial_constrained_hyper_area * (is_orientationally_constrained() ? MIN(total_limit_cone_surface_area_ratio, 1) : 1);
}

void IKKusudama::set_strength(double newStrength) {
	this->strength = MAX(0, MIN(1, newStrength));
}

double IKKusudama::get_strength() const {
	return this->strength;
}

Vector<Ref<LimitCone>> IKKusudama::get_limit_cones() const {
	return limit_cones;
}

Vector3 IKKusudama::local_point_on_path_sequence(Vector3 in_point, Ref<IKTransform3D> limiting_axes) {
	double closest_point_dot = 0;
	Vector3 point = limiting_axes->get_transform().xform(in_point);
	point.normalize();
	Vector3 result = point;

	if (limit_cones.size() == 1) {
		result = limit_cones[0]->get_control_point();
	} else {
		for (int i = 0; i < limit_cones.size() - 1; i++) {
			Ref<LimitCone> next_cone = limit_cones[i + 1];
			Vector3 closestPathPoint = limit_cones[i]->get_closest_path_point(next_cone, point);
			double closeDot = closestPathPoint.dot(point);
			if (closeDot > closest_point_dot) {
				result = closestPathPoint;
				closest_point_dot = closeDot;
			}
		}
	}

	return result;
}

/**
 * Given a point (in global coordinates), checks to see if a ray can be extended from the Kusudama's
 * origin to that point, such that the ray in the Kusudama's reference frame is within the range allowed by the Kusudama's
 * coneLimits.
 * If such a ray exists, the original point is returned (the point is within the limits).
 * If it cannot exist, the tip of the ray within the kusudama's limits that would require the least rotation
 * to arrive at the input point is returned.
 * @param in_point the point to test.
 * @param in_bounds returns a number from -1 to 1 representing the point's distance from the boundary, 0 means the point is right on
 * the boundary, 1 means the point is within the boundary and on the path furthest from the boundary. any negative number means
 * the point is outside of the boundary, but does not signify anything about how far from the boundary the point is.
 * @return the original point, if it's in limits, or the closest point which is in limits.
 */
Vector3 IKKusudama::_local_point_in_limits(Vector3 in_point, Vector<double> &in_bounds, int mode) {
	Vector3 point = in_point.normalized();
	real_t closest_cos = -2.0;
	Vector3 closest_collision_point = Vector3(NAN, NAN, NAN);
	// This is an exact check for being inside the bounds.
	for (int i = 0; i < limit_cones.size(); i++) {
		Ref<LimitCone> cone = limit_cones[i];
		Ref<LimitCone> cone_next;
		if (i - 1 > -1) {
			cone_next = limit_cones[i - 1];
		}
		bool is_in_bounds = cone->determine_if_in_bounds(cone_next, point);
		if (is_in_bounds) {
			in_bounds.write[0] = 1;
			return point;
		}
		Vector3 collision_point = cone->closest_to_cone(point, in_bounds);
		if (Math::is_nan(collision_point.x)) {
			in_bounds.write[0] = 1;
			return point;
		}
		real_t this_cos = collision_point.dot(point);
		if (Math::is_nan(closest_collision_point.x) || this_cos > closest_cos) {
			closest_collision_point = collision_point;
			closest_cos = this_cos;
		}
	}
	// Case where there are multiple cones and we're out of bounds of all cones.
	// Are we in the paths between the cones.
	for (int i = 0; i < limit_cones.size() - 1; i++) {
		Ref<LimitCone> currCone = limit_cones[i];
		Ref<LimitCone> nextCone = limit_cones[i + 1];
		Vector3 collision_point = currCone->get_on_great_tangent_triangle(nextCone, point);
		if (!Math::is_nan(collision_point.x)) {
			continue;
		}
		real_t this_cos = collision_point.dot(point);
		if (Math::is_equal_approx(this_cos, real_t(1.0))) {
			in_bounds.write[0] = 1;
			return point;
		}
		if (this_cos > closest_cos) {
			closest_collision_point = collision_point;
			closest_cos = this_cos;
		}
	}
	// Move to the closest boundary between cones or on the cones if out of bounds.
	return closest_collision_point;
}

void IKKusudama::set_axes_to_orientation_snap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double p_dampening, double p_cos_half_angle_dampen) {
	Vector<double> in_bounds = { 1 };
	bone_ray->p1(limiting_axes->get_global_transform().origin);
	bone_ray->p2(to_set->get_global_transform().basis.get_column(Vector3::AXIS_Y));
	Vector3 bone_tip = limiting_axes->to_local(bone_ray->p2());
	Vector3 in_limits = _local_point_in_limits(bone_tip, in_bounds, IKKusudama::BOUNDARY);
	if (in_bounds[0] < 0 && !Math::is_nan(in_limits.x)) {
		constrained_ray->p1(bone_ray->p1());
		constrained_ray->p2(limiting_axes->to_global(in_limits));
		Quaternion rectified_rot = Quaternion(bone_ray->heading(), constrained_ray->heading());
		to_set->rotate_local_with_global(rectified_rot);
		_ALLOW_DISCARD_ to_set->get_global_transform();
	}
}

void IKKusudama::set_axes_to_soft_orientation_snap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen) {
}

Ref<IKTransform3D> IKKusudama::limiting_axes() {
	// if(inverted) return inverseLimitingAxes;
	return _limiting_axes;
}
