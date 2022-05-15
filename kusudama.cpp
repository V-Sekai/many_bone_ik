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

#include "kusudama.h"
#include "math/ik_transform.h"

IKKusudama::IKKusudama() {
}

IKKusudama::IKKusudama(Ref<IKBone3D> for_bone) {
	this->_attached_to = for_bone;
	this->_limiting_axes->set_global_transform(for_bone->get_global_pose());
	this->_attached_to->addConstraint(this);
	this->enable();
}

void IKKusudama::_update_constraint() {
	this->update_tangent_radii();
	this->update_rotational_freedom();
}

void IKKusudama::optimize_limiting_axes() {
	Ref<IKTransform3D> originalLimitingAxes;
	originalLimitingAxes.instantiate();
	originalLimitingAxes->set_global_transform(_limiting_axes->get_global_transform());
	Vector<Vector3> directions;
	if (get_limit_cones().size() == 1) {
		directions.push_back(limit_cones[0]->get_control_point());
	} else {
		for (int i = 0; i < get_limit_cones().size() - 1; i++) {
			Vector3 thisC = get_limit_cones()[i]->get_control_point();
			Vector3 nextC = get_limit_cones()[i + 1]->get_control_point();
			Basis thisToNext = Basis(thisC, nextC);
			Vector3 axis;
			real_t angle;
			thisToNext.get_axis_angle(axis, angle);
			Basis halfThisToNext(axis, angle / 2.0f);

			Vector3 halfAngle = halfThisToNext.xform(thisC);
			halfAngle.normalize();
			halfAngle *= thisToNext.get_euler();
			directions.push_back(halfAngle);
		}
	}

	Vector3 newY;
	for (Vector3 dv : directions) {
		newY += dv;
	}

	newY /= directions.size();
	if (newY.length() != 0.f && !Math::is_nan(newY.y)) {
		newY.normalize();
	} else {
		newY = Vector3(0.f, 1.f, 0.f);
	}

	Vector3 temp_var(0.f, 0.f, 0.f);
	Ref<Ray3D> newYRay = memnew(Ray3D(temp_var, newY));

	Quaternion oldYtoNewY = quaternion_unnormalized(_limiting_axes->get_global_transform().basis[Vector3::AXIS_Y], 
		originalLimitingAxes->to_global(newYRay->heading()));
	_limiting_axes->rotate_by(oldYtoNewY);

	for (Ref<LimitCone> lc : get_limit_cones()) {
		lc->control_point = originalLimitingAxes->to_global(lc->control_point);
		lc->control_point = _limiting_axes->to_local(lc->control_point);
		lc->control_point.normalize();
	}

	this->update_tangent_radii();
}

void IKKusudama::update_tangent_radii() {
	for (int i = 0; i < limit_cones.size(); i++) {
		Ref<LimitCone> next = i < limit_cones.size() - 1 ? limit_cones[i + 1] : nullptr;
		limit_cones.write[i]->update_tangent_handles(next);
	}
}

Quaternion IKKusudama::set_axes_to_snapped(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen) {
	Quaternion rot;
	if (limiting_axes != nullptr) {
		if (orientationally_constrained) {
			rot = rot * set_axes_to_orientation_snap(to_set, limiting_axes, cos_half_angle_dampen);
		}
		if (axially_constrained) {
			double twist_diff = 0.0;
			rot = rot * snap_to_twist_limits(to_set, limiting_axes, twist_diff);
		}
	}
	return rot;
}

// Todo: fire 2022-05-13 Haven't been using this code path the last week. This is planned to be replaced by cushions.
// Re-enable and debug if we want bouncy constraint.
// void IKKusudama::setAxesToReturnfulled(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double cosHalfReturnfullness, double angleReturnfullness) {
// 	if (limiting_axes != nullptr && painfullness > 0) {
// 		if (orientationally_constrained) {
// 			Vector3 origin = to_set->get_transform().origin;
// 			Vector3 in_point = to_set->get_transform().basis[Vector3::AXIS_Y];
// 			Vector3 pathPoint = point_on_path_sequence(in_point, limiting_axes);
// 			in_point -= origin;
// 			pathPoint -= origin;
// 			Quaternion toClamp = Quaternion(in_point, pathPoint);
// 			toClamp.rotation.clampToQuadranceAngle(cosHalfReturnfullness);
// 			to_set->rotate_by(toClamp);
// 		}
// 		if (axially_constrained) {
// 			double angleToTwistMid = angle_to_twist_center(to_set, limiting_axes);
// 			double clampedAngle = CLAMP(angleToTwistMid, -angleReturnfullness, angleReturnfullness);
// 			to_set->rotateAboutY(clampedAngle, false);
// 		}
// 	}
// }

// void IKKusudama::set_painfullness(double amt) {
// 	painfullness = amt;
// 	if (!(attached_to().is_valid() && attached_to()->parentArmature.is_valid())) {
// 		return;
// 	}
// 	Ref<IKBone3D> wb = this->attached_to();
// 	if (wb.is_null()) {
// 		return;
// 	}
// 	wb->updateCosDampening();
// }

// double IKKusudama::getPainfullness() {
// 	return painfullness;
// }

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
	 *
	 * Because we can expect rotations to be fairly small, we use nlerp instead of slerp for efficiency when averaging.
	 */
	bone_ray->p1(limiting_axes->get_global_transform().origin);
	bone_ray->p2(bone_direction->get_global_transform().basis[Vector3::AXIS_Y]);
	Vector3 bone_tip = limiting_axes->to_local(bone_ray->p2());
	Vector3 inCushionLimits = point_in_limits(bone_tip, in_bounds, IKKusudama::CUSHION);

	if (in_bounds[0] == -1 && inCushionLimits != Vector3(NAN, NAN, NAN)) {
		constrained_ray->p1(bone_ray->p1());
		constrained_ray->p2(limiting_axes->to_global(inCushionLimits));
		Quaternion rectified_rotation = quaternion_unnormalized(bone_ray->heading(), constrained_ray->heading());
		to_set->rotate_by(rectified_rotation);
	}
}

bool IKKusudama::is_in_orientation_limits(Ref<IKTransform3D> global_axes, Ref<IKTransform3D> limiting_axes) {
	Vector<double> in_bounds = { 1 };
	Vector3 local_point = _limiting_axes->to_local(global_axes->get_global_transform().basis[Vector3::AXIS_Y]);
	_ALLOW_DISCARD_ point_in_limits(local_point.normalized(),
			in_bounds);
	if (in_bounds[0] == -1) {
		return false;
	}
	return true;
}

void IKKusudama::set_axial_limits(double min_angle, double in_range) {
	_min_axial_angle_conflict = min_angle;
	range = to_tau(in_range);
	_update_constraint();
}

Quaternion IKKusudama::snap_to_twist_limits(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double &r_turn_diff) {
	if (!axially_constrained) {
		r_turn_diff = 0;
		return Quaternion();
	}
	Basis inv_rot = limiting_axes->get_global_transform().basis.inverse();
	Basis align_rot = inv_rot * to_set->get_global_transform().basis;
	Vector3 temp_var(0, 1, 0);
	Vector<Quaternion> decomposition = get_swing_twist(align_rot, temp_var);
	double angle_delta_2 = decomposition[1].get_angle() * decomposition[1].get_axis().y * -1;
	angle_delta_2 = to_tau(angle_delta_2);
	double from_min_to_angle_delta = to_tau(signed_angle_difference(angle_delta_2, Math_TAU - this->min_axial_angle()));

	Quaternion quaternion;
	if (from_min_to_angle_delta < Math_TAU - range) {
		double dist_to_min = Math::abs(signed_angle_difference(angle_delta_2, Math_TAU - this->min_axial_angle()));
		double dist_to_max = Math::abs(signed_angle_difference(angle_delta_2, Math_TAU - (this->min_axial_angle() + range)));
		double turnDiff = 1;
		// TODO: fire 2022-05-13 restore chirality
		// turnDiff *= limiting_axes->getGlobalChirality();
		Vector3 axis = to_set->get_global_transform().basis[Vector3::AXIS_Y].normalized();
		if (dist_to_min < dist_to_max) {
			turnDiff = turnDiff * (from_min_to_angle_delta);
			quaternion = Quaternion(axis, turnDiff);
		} else {
			turnDiff = turnDiff * (range - (Math_TAU - from_min_to_angle_delta));
			quaternion = Quaternion(axis, turnDiff);
			to_set->rotate_by(quaternion);
		}
		r_turn_diff = turnDiff < 0 ? turnDiff * -1 : turnDiff;
		return quaternion;
	}
	r_turn_diff = 0;
	return quaternion;
}

double IKKusudama::angle_to_twist_center(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes) {
	if (!axially_constrained) {
		return 0;
	}

	Quaternion align_rot = limiting_axes->get_global_transform().basis.inverse() * to_set->get_global_transform().basis;
	Vector3 temp_var(0, 1, 0);
	Vector<Quaternion> decomposition = get_swing_twist(align_rot, temp_var);
	double angle_delta_2 = decomposition[1].get_angle() * Basis(decomposition[1]).get_euler().y * -1;
	angle_delta_2 = to_tau(angle_delta_2);

	double distToMid = signed_angle_difference(angle_delta_2, Math_TAU - (this->min_axial_angle() + (range / 2)));
	return distToMid;
}

bool IKKusudama::in_twist_limits(Ref<IKTransform3D> bone_axes, Ref<IKTransform3D> limiting_axes) {
	Basis inv_rot = limiting_axes->get_global_transform().basis.inverse();
	Basis align_rot = inv_rot * bone_axes->get_global_transform().basis;
	Vector3 temp_var(0, 1, 0);
	Vector<Quaternion> decomposition = get_swing_twist(align_rot, temp_var);
	double angle_delta_2 = decomposition[1].get_angle() * decomposition[1].get_axis().y * -1;
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

void IKKusudama::add_limit_cone(Vector3 new_point, double radius, Ref<LimitCone> previous, Ref<LimitCone> next) {
	int insert_at = 0;

	if (next.is_null() || limit_cones.is_empty()) {
		add_limit_cone_at_index(0, new_point, radius);
	} else if (previous.is_valid()) {
		insert_at = limit_cones.find(previous) + 1;
	} else {
		insert_at = MAX(0, limit_cones.find(next));
	}
	add_limit_cone_at_index(insert_at, new_point, radius);
}

void IKKusudama::remove_limit_cone(Ref<LimitCone> limitCone) {
	this->limit_cones.erase(limitCone);
	this->update_tangent_radii();
	this->update_rotational_freedom();
}

void IKKusudama::add_limit_cone_at_index(int insert_at, Vector3 new_point, double radius) {
	Ref<LimitCone> newCone = memnew(LimitCone(new_point, radius, Ref<IKKusudama>(this)));
	limit_cones.insert(insert_at, newCone);
	this->update_tangent_radii();
	this->update_rotational_freedom();
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
	return _min_axial_angle_conflict;
}

double IKKusudama::max_axial_angle() {
	return range;
}

double IKKusudama::absolute_max_axial_angle() {
	return signed_angle_difference(range + _min_axial_angle_conflict, Math_PI * 2);
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
	for (auto l : limit_cones) {
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

Vector<Ref<LimitCone>> IKKusudama::get_limit_cones() {
	return limit_cones;
}

bool IKKusudama::_is_in_limits(Vector3 global_point) {
	Vector<double> in_bounds = { 1 };
	Vector3 local_point = _limiting_axes->to_local(global_point);
	_ALLOW_DISCARD_ this->point_in_limits(local_point.normalized(), in_bounds, LimitCone::BOUNDARY);
	return in_bounds[0] > 0;
}

Vector<Quaternion> IKKusudama::get_swing_twist(Quaternion p_quaternion, Vector3 p_axis) {
	Quaternion twist_rotation = p_quaternion;
	const float d = twist_rotation.get_axis().dot(p_axis);
	twist_rotation = Quaternion(p_axis.x * d, p_axis.y * d, p_axis.z * d, twist_rotation.w);
	if (d < 0) {
		twist_rotation *= -1.0f;
	}
	Quaternion swing = twist_rotation;
	swing.x = -swing.x;
	swing.y = -swing.y;
	swing.z = -swing.z;
	swing = twist_rotation * swing;

	Vector<Quaternion> result;
	result.resize(2);
	result.write[0] = swing;
	result.write[1] = twist_rotation;
	return result;
}

Vector3 IKKusudama::point_on_path_sequence(Vector3 in_point, Ref<IKTransform3D> limiting_axes) {
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

	return limiting_axes->get_global_transform().xform(result);
}

Vector3 IKKusudama::point_in_limits(Vector3 in_point, Vector<double> &in_bounds, int mode) {
	Vector3 point = in_point;
	point.normalize();

	in_bounds.write[0] = -1;
	Vector3 closest_collision_point = Vector3(NAN, NAN, NAN);
	double closest_cos = -2;
	Vector<bool> bound_hint = { false };

	for (int i = 0; i < limit_cones.size(); i++) {
		Ref<LimitCone> cone = limit_cones[i];
		Vector3 collision_point = cone->closest_to_cone(point, bound_hint);
		if (Math::is_nan(collision_point.x) || Math::is_nan(collision_point.y) || Math::is_nan(collision_point.z)) {
			in_bounds.write[0] = 1;
			return point;
		} else {
			double this_cos = collision_point.dot(point);
			if (Math::is_nan(collision_point.x) || Math::is_nan(collision_point.y) || Math::is_nan(collision_point.z) || this_cos > closest_cos) {
				closest_collision_point = collision_point;
				closest_cos = this_cos;
			}
		}
	}
	if (in_bounds[0] == -1) {
		for (int i = 0; i < limit_cones.size() - 1; i++) {
			Ref<LimitCone> curr_cone = limit_cones[i];
			Ref<LimitCone> next_cone = limit_cones[i + 1];
			Vector3 collision_point = curr_cone->get_on_great_tangent_triangle(next_cone, point);
			if (!(Math::is_nan(collision_point.x) || Math::is_nan(collision_point.y) || Math::is_nan(collision_point.z))) {
				double this_cos = collision_point.dot(point);
				if (Math::is_equal_approx(this_cos, 1.0)) {
					in_bounds.write[0] = 1;
					closest_collision_point = point;
					return point;
				} else if (this_cos > closest_cos) {
					closest_collision_point = collision_point;
					closest_cos = this_cos;
				}
			}
		}
	}

	return closest_collision_point;
}

Quaternion IKKusudama::set_axes_to_orientation_snap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen) {
	Vector<double> in_bounds = { 1 };
	bone_ray->p1(limiting_axes->get_global_transform().origin);
	bone_ray->p2(to_set->get_global_transform().basis[Vector3::AXIS_Y]);
	Vector3 bone_tip = limiting_axes->to_local(bone_ray->p2());
	Vector3 in_limits = this->point_in_limits(bone_tip, in_bounds);
	Quaternion rectified_rotation;
	if (in_bounds[0] == -1 && in_limits != Vector3(NAN, NAN, NAN)) {
		constrained_ray->p1(bone_ray->p1());
		constrained_ray->p2(limiting_axes->to_global(in_limits));
		rectified_rotation = quaternion_unnormalized(bone_ray->heading(), constrained_ray->heading());
	}
	return rectified_rotation;
}

Quaternion IKKusudama::set_axes_to_soft_orientation_snap(Ref<IKTransform3D> to_set, Ref<IKTransform3D> bone_direction, Ref<IKTransform3D> limiting_axes, double cos_half_angle_dampen) {
	return set_axes_to_orientation_snap(to_set, limiting_axes, cos_half_angle_dampen);
}

Quaternion IKKusudama::quaternion_unnormalized(Vector3 u, Vector3 v) {
	float norm_product = u.length() * v.length();
	Quaternion ret;
	if (Math::is_zero_approx(norm_product)) {
		return ret;
	}
	float dot = u.dot(v);
	if (dot < ((2.0e-15 - 1.0f) * norm_product)) {
		// The special case u = -v: we select a PI angle rotation around
		// an arbitrary vector orthogonal to u.
		Vector3 w = LimitCone::get_orthogonal(u);
		ret.w = 0.0f;
		ret.x = -w.x;
		ret.y = -w.y;
		ret.z = -w.z;
	}
	return Quaternion(u, v);
}