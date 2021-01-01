/*************************************************************************/
/*  ik_constraint_kusudama.cpp                                           */
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

#include "kusudama_constraint.h"
#include "core/object/object.h"
#include "direction_constraint.h"
#include "quat_ik.h"
#include "skeleton_modification_3d_ewbik.h"

void KusudamaConstraint::optimize_limiting_axes() {
	const Transform original_constraint_axes = get_constraint_axes();

	Vector<Vector3> directions;
	if (multi_direction.size() == 1) {
		ERR_FAIL_INDEX(1, multi_direction.size());
		Ref<DirectionConstraint> direction_limit = multi_direction[0];
		directions.push_back(directions.write[0] + direction_limit->get_control_point().normalized());
	} else {
		for (int limit_i = 0; limit_i < multi_direction.size() - 1; limit_i++) {
			Ref<DirectionConstraint> direction_limit_0 = multi_direction[limit_i];
			Vector3 this_c = direction_limit_0->get_control_point().normalized();
			Ref<DirectionConstraint> direction_limit_1 = multi_direction[limit_i + 1];
			Vector3 next_c = direction_limit_1->get_control_point().normalized();
			Quat this_to_next = Quat(this_c, next_c);
			Vector3 axis;
			real_t angle;
			Basis(this_to_next).get_axis_angle(axis, angle);
			Quat half_this_to_next = Quat(axis, angle / 2.0f);

			Vector3 half_angle = Basis(half_this_to_next).rotated(this_c).get_euler();
			half_angle.normalize();
			half_angle *= this_to_next.get_euler();
			directions.push_back(half_angle);
		}
	}

	Vector3 newY;
	for (int32_t direction_i = 0; direction_i < directions.size();
			direction_i++) {
		newY += directions[direction_i];
	}

	newY /= directions.size();
	if (newY.length() != 0 && !Math::is_nan(newY.y)) {
		newY.normalize();
	} else {
		newY = Vector3(0.0, 1.0f, 0.0);
	}

	Ray newYRay = Ray(Vector3(0.0, 0.0, 0.0), newY);

	// TODO
	// Quat old_y_to_new_y = Quat(limiting_axes.y_().heading(), original_limiting_axes.getGlobalOf(newYRay).heading());
	Transform xform;
	xform.basis = original_constraint_axes.get_basis();
	Quat old_y_to_new_y = get_constraint_axes().get_basis() * xform.get_basis();
	get_constraint_axes().basis.rotate(old_y_to_new_y);
	for (int32_t direction_limit_i = 0; direction_limit_i < multi_direction.size(); direction_limit_i++) {
		Ref<DirectionConstraint> direction_limit = multi_direction[direction_limit_i];
		// original_limiting_axes.setToGlobalOf(direction_limit->get_control_point(), direction_limit->get_control_point().normalized());
		// limiting_axes.setToLocalOf(direction_limit->get_control_point(), direction_limit->get_control_point().normalized());

		direction_limit->get_control_point().normalize();
	}
	update_tangent_radii();
	_change_notify();
}

void KusudamaConstraint::set_twist_limits(float p_min_angle, float p_in_range) {
	twist->set_min_twist_angle(p_min_angle);
	twist->set_range(p_in_range);
	constraint_update_notification();
}

void KusudamaConstraint::add_direction_constraint_at_index(int p_insert_at, Vector3 p_new_point, float p_radius) {
	Ref<DirectionConstraint> new_direction_limit = create_direction_limit_for_index(p_insert_at, p_new_point, p_radius);
	ERR_FAIL_COND(new_direction_limit.is_null());
	if (p_insert_at == -1) {
		multi_direction.push_back(new_direction_limit);
	} else {
		multi_direction.insert(p_insert_at, new_direction_limit);
	}
	update_tangent_radii();
	update_rotational_freedom();
}

Ref<DirectionConstraint>
KusudamaConstraint::create_direction_limit_for_index(int p_insert_at, Vector3 p_new_point, float p_radius) {
	ERR_FAIL_INDEX_V(p_insert_at, multi_direction.size(), NULL);
	Ref<DirectionConstraint> direction_limit;
	direction_limit.instance();
	direction_limit->initialize(p_new_point, p_radius, this);
	multi_direction.write[p_insert_at] = direction_limit;
	return direction_limit;
}

real_t KusudamaConstraint::get_rotational_freedom() const {
	return rotational_freedom;
}

real_t KusudamaConstraint::from_tau(real_t p_tau) const {
	real_t result = p_tau;

	if (p_tau < 0.0f) {
		result = 1.0f + p_tau;
	}
	result = Math::fmod(result, 1.0f);
	result = result * Math_PI;
	return result;
}

Vector3 KusudamaConstraint::point_in_limits(Vector3 inPoint, Vector<real_t> inBounds, Transform limitingAxes) {
	Vector3 point = inPoint;
	//TODO
	// limitingAxes.setToLocalOf(inPoint, point);
	point.normalize();

	inBounds.write[0] = -1;

	Vector3 closestCollisionPoint;
	float closestCos = -2.0f;
	if (multi_direction.size() > 1 && orientation_constrained) {
		for (int i = 0; i < multi_direction.size() - 1; i++) {
			Vector3 collisionPoint;
			Ref<DirectionConstraint> nextCone = multi_direction[i + 1];
			bool inSegBounds = multi_direction[i]->in_bounds_from_this_to_next(nextCone, point, collisionPoint);
			if (inSegBounds == true) {
				inBounds.write[0] = 1.0;
			} else {
				float thisCos = collisionPoint.dot(point);
				if (closestCollisionPoint == Vector3() || thisCos > closestCos) {
					closestCollisionPoint = collisionPoint;
					closestCos = thisCos;
				}
			}
		}
		if (inBounds[0] == -1) {
			//TODO
			//return limitingAxes.getGlobalOf(closestCollisionPoint);
			return closestCollisionPoint;
		} else {
			return inPoint;
		}
	} else if (orientation_constrained) {
		if (point.dot(multi_direction[0]->get_control_point()) > multi_direction[0]->get_radius_cosine()) {
			inBounds.write[0] = 1;
			return inPoint;
		} else {
			// Vector3 axis = multi_direction[0]->get_control_point().cross(point);
			// Quat toLimit = Quat(axis, multi_direction[0]->get_radius());
			//TODO
			//return limitingAxes.getGlobalOf(toLimit.applyToCopy(limitCones.get(0).getControlPoint()));
			return multi_direction[0]->get_control_point();
		}
	} else {
		inBounds.write[0] = 1.0f;
		return inPoint;
	}
}

real_t KusudamaConstraint::to_tau(real_t p_angle) const {
	real_t result = p_angle;
	if (p_angle < 0) {
		result = (2 * Math_PI) + p_angle;
	}
	result = Math::fmod(double(result), Math_PI * 2.0);
	return result;
}

real_t KusudamaConstraint::signed_angle_difference(real_t p_min_angle, real_t p_base) {
	real_t d = Math::fmod(Math::absd(p_min_angle - p_base), double(Math_TAU));
	real_t r = d > Math_PI ? Math_TAU - d : d;
	real_t sign = (p_min_angle - p_base >= 0 && p_min_angle - p_base <= Math_PI) ||
								  (p_min_angle - p_base <= -Math_PI && p_min_angle - p_base >= -Math_TAU) ?
						  1.0f :
						  -1.0f;
	r *= sign;
	return r;
}

real_t KusudamaConstraint::snap_to_twist_limits(Transform p_to_set, Transform p_limiting_axes) {	
	if (!is_enabled()) {
		return 0.0f;
	}
	if (!axial_constrained) {
		return 0.0f;
	}

	QuatIK alignRot = p_limiting_axes.basis.get_rotation_quat().inverse() * p_to_set.basis.get_rotation_quat();
	Vector<QuatIK> decomposition = alignRot.get_swing_twist(Vector3(0, 1, 0));
	const int32_t axis_y = 1;
	real_t angleDelta2 = Basis(decomposition[1]).get_axis(axis_y).y * -1.0f;
	angleDelta2 = to_tau(angleDelta2);
	real_t fromMinToAngleDelta = to_tau(signed_angle_difference(angleDelta2, twist->get_min_twist_angle()));

	if (fromMinToAngleDelta < Math_TAU - to_tau(twist->get_range())) {
		real_t distToMin = Math::absf(signed_angle_difference(angleDelta2, twist->get_min_twist_angle()));
		real_t distToMax = Math::absf(signed_angle_difference(angleDelta2, Math_TAU - (to_tau(
																							   twist->get_min_twist_angle()) +
																							  to_tau(twist->get_range()))));
		real_t turnDiff = 1.0f;
		// Chirality
		turnDiff *= 1;
		if (distToMin < distToMax) {
			turnDiff = turnDiff * (fromMinToAngleDelta);
			p_to_set.rotate_basis(Vector3(0, 1, 0), turnDiff);
			p_to_set.orthonormalize();
		} else {
			turnDiff = turnDiff * (to_tau(twist->get_range()) - (Math_TAU - fromMinToAngleDelta));
			p_to_set.rotate_basis(Vector3(0, 1, 0), turnDiff);
			p_to_set.orthonormalize();
		}
		return turnDiff < 0 ? turnDiff * -1 : turnDiff;
	} else {
		return 0.f;
	}
	return 0.f;
}

void KusudamaConstraint::update_tangent_radii() {
	for (int direction_limit_i = 0; direction_limit_i < multi_direction.size(); direction_limit_i++) {
		Ref<DirectionConstraint> next;
		if (direction_limit_i < multi_direction.size() - 1) {
			next = multi_direction[direction_limit_i + 1];
		}
		if (next.is_null()) {
			continue;
		}
		Ref<DirectionConstraint> direction_limit = multi_direction[direction_limit_i];
		direction_limit->update_tangent_handles(next);
	}
}

void KusudamaConstraint::constraint_update_notification() {
	if(!is_enabled()) {
		return;
	}
	update_tangent_radii();
	update_rotational_freedom();
}

void KusudamaConstraint::set_axes_to_snapped(Transform p_to_set, Transform p_limiting_axes, float p_cos_half_angle_dampen) {
	if (!p_limiting_axes.is_equal_approx(Transform())) {
		if (orientation_constrained) {
			set_axes_to_orientation_snap(p_to_set, p_limiting_axes, p_cos_half_angle_dampen);
		}
		if (axial_constrained) {
			snap_to_twist_limits(p_to_set, p_limiting_axes);
		}
	}
}

void KusudamaConstraint::set_axes_to_orientation_snap(Transform p_to_set, Transform p_limiting_axes,
		float p_cos_half_angle_dampen) {
	Vector<real_t> inBounds;
	inBounds.push_back(1.f);
	bone_ray.position = Vector3(p_to_set.origin);

	// toSet.y_().getScaledTo(attachedTo.boneHeight);
	bone_ray.normal = p_to_set.basis.get_axis(y_axis) * attached_to->mod->get_state()->get_height(attached_to->bone);
	// Vector3 in_limits = point_in_limits(bone_ray.normal, inBounds, limiting_axes);

	// if (inBounds[0] == -1 && inLimits != Vector3()) {
	// 	constrained_ray.position = bone_ray.position;
	// 	constrained_ray.normal = inLimits;
	// 	Quat rectifiedRot = Quat(bone_ray.heading(), constrained_ray.heading());
	// 	toSet.basis *= rectifiedRot;
	// }
}

void KusudamaConstraint::set_axes_to_returnful(Transform p_global_xform, Transform p_to_set, Transform p_limiting_axes,
		real_t p_cos_half_angle_dampen, real_t p_angle_dampen) {
	if (!p_limiting_axes.is_equal_approx(Transform()) && pain > 0.0f) {
		const int32_t y_axis = 1;
		if (orientation_constrained) {
			Vector3 origin = p_to_set.origin;
			Vector3 inPoint = p_to_set.basis.get_axis(y_axis);
			Vector3 pathPoint = point_on_path_sequence(p_global_xform, inPoint, p_limiting_axes);
			inPoint -= origin;
			pathPoint -= origin;
			QuatIK toClamp = Quat(inPoint, pathPoint);
			toClamp.clamp_to_quadrance_angle(p_cos_half_angle_dampen);
			p_to_set.basis.rotate(toClamp);
		}
		if (axial_constrained) {
			real_t angleToTwistMid = angle_to_twist_center(p_global_xform, p_to_set, p_limiting_axes);
			real_t clampedAngle = CLAMP(angleToTwistMid, -p_angle_dampen, p_angle_dampen);
			p_to_set.basis = Quat(p_global_xform.basis.get_axis(y_axis), clampedAngle);
		}
	}
}

real_t KusudamaConstraint::angle_to_twist_center(Transform p_global_xform, Transform p_to_set, Transform p_limiting_axes) {
	if (!axial_constrained) {
		return 0.0f;
	}
	Transform constraint_axes_global = p_global_xform * get_constraint_axes();
	Transform to_set_global = p_global_xform * p_to_set;
	QuatIK align_rot =
			constraint_axes_global.basis.get_rotation_quat().inverse() * to_set_global.basis.get_rotation_quat();
	Vector<QuatIK> decomposition = align_rot.get_swing_twist(Vector3(0, 1, 0));
	const int32_t axis_y = 1;
	real_t angle_delta_2 = Basis(decomposition[1]).get_axis(axis_y).y * -1.0f;
	angle_delta_2 = to_tau(angle_delta_2);
	real_t dist_to_mid = signed_angle_difference(angle_delta_2,
			Math_TAU - (to_tau(twist->get_min_twist_angle()) + (to_tau((twist->get_range()) / 2.0f))));
	return dist_to_mid;
}

Vector3
KusudamaConstraint::point_on_path_sequence(Transform p_global_xform, Vector3 p_in_point, Transform p_limiting_axes) {
	real_t closestPointDot = 0.0f;
	Vector3 point = p_limiting_axes.origin;
	point.normalize();
	Vector3 result = point;

	if (multi_direction.size() == 1) {
		Ref<DirectionConstraint> direction_limit = multi_direction[0];
		result = direction_limit->control_point;
	} else {
		for (int direction_limit_i = 0; direction_limit_i < multi_direction.size() - 1; direction_limit_i++) {
			Ref<DirectionConstraint> next_direction = multi_direction[direction_limit_i + 1];
			Ref<DirectionConstraint> direction = multi_direction[direction_limit_i];
			Vector3 closest_path_point = direction->get_closest_path_point(next_direction, point);
			real_t close_dot = closest_path_point.dot(point);
			if (close_dot > closestPointDot) {
				result = closest_path_point;
				closestPointDot = close_dot;
			}
		}
	}

	return p_global_xform.xform(result);
}

real_t KusudamaConstraint::get_pain() {
	return pain;
}

void KusudamaConstraint::disable() {
	enabled = false;
}

void KusudamaConstraint::enable() {
	enabled = true;
}

bool KusudamaConstraint::is_enabled() const {
	return enabled;
}

bool KusudamaConstraint::is_in_limits_(const Vector3 p_global_point) const {
	return false;
}

void KusudamaConstraint::update_rotational_freedom() {
	float axialConstrainedHyperArea = axial_constrained ? (to_tau(twist->get_range()) / Math_TAU) : 1.0f;
	// quick and dirty solution (should revisit);
	float total_limit_cone_surface_area_ratio = 0.0f;
	ERR_FAIL_INDEX(direction_count - 1, multi_direction.size());
	for (int32_t limit_cone_i = 0; limit_cone_i < direction_count; limit_cone_i++) {
		Ref<DirectionConstraint> direction_limit = multi_direction[limit_cone_i];
		ERR_CONTINUE(direction_limit.is_null());
		total_limit_cone_surface_area_ratio += (direction_limit->get_radius() * 2.0f) / Math_TAU;
	}
	rotational_freedom = axialConstrainedHyperArea *
						 (orientation_constrained ? MIN(total_limit_cone_surface_area_ratio, 1.0f) : 1.0f);
}

void KusudamaConstraint::set_pain(real_t p_amount) {
	pain = p_amount;
	if (attached_to.is_null() || attached_to->chain_root == NULL) {
		return;
	}
	Ref<EWBIKSegmentedSkeleton3D> s = attached_to->chain_root;
	if (s.is_null()) {
		return;
	}
	Ref<EWBIKSegmentedSkeleton3D> wb = s->chain_root->find_child(attached_to->bone);
	if (wb.is_null()) {
		return;
	}
	// wb->update_cos_dampening();
	_change_notify();
}

void KusudamaConstraint::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_constraint_axes", "axes"), &KusudamaConstraint::set_constraint_axes);
	ClassDB::bind_method(D_METHOD("get_constraint_axes"), &KusudamaConstraint::get_constraint_axes);
	ClassDB::bind_method(D_METHOD("set_twist_constraint", "twist"), &KusudamaConstraint::set_twist_constraint);
	ClassDB::bind_method(D_METHOD("get_twist_constraint"), &KusudamaConstraint::get_twist_constraint);
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "constraint_axes"), "set_constraint_axes", "get_constraint_axes");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "twist", PROPERTY_HINT_RESOURCE_TYPE, "IKTwistConstraint"), "set_twist_constraint", "get_twist_constraint");
}

Transform KusudamaConstraint::get_constraint_axes() const {
	return constraint_axes;
}

void KusudamaConstraint::set_constraint_axes(Transform p_axes) {
	constraint_axes = p_axes;
	_change_notify();
}

int32_t KusudamaConstraint::get_direction_count() const {
	return direction_count;
}

void KusudamaConstraint::set_direction_count(int32_t p_count) {
	direction_count = p_count;
	multi_direction.resize(p_count);
	_change_notify();
}

void KusudamaConstraint::set_direction(int32_t p_index, Ref<DirectionConstraint> p_constraint) {
	ERR_FAIL_INDEX(p_index, multi_direction.size());
	multi_direction.write[p_index] = p_constraint;
	_change_notify();
}

void KusudamaConstraint::remove_direction(int32_t p_index) {
	ERR_FAIL_INDEX(p_index, multi_direction.size());
	multi_direction.remove(p_index);
	set_direction_count(direction_count - 1);
	_change_notify();
}

KusudamaConstraint::KusudamaConstraint(Ref<EWBIKSegmentedSkeleton3D> p_for_bone) {
	twist.instance();
	attached_to = p_for_bone;
	//     limiting_axes = p_for_bone.getMajorRotationAxes();
	//     attached_to->parent_armature->addConstraint(this);
	enable();
}

KusudamaConstraint::KusudamaConstraint() {
}

KusudamaConstraint::~KusudamaConstraint() {
}

Ref<DirectionConstraint> KusudamaConstraint::get_direction(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, multi_direction.size(), NULL);
	return multi_direction[p_index];
}

Ref<TwistConstraint> KusudamaConstraint::get_twist_constraint() const {
	return twist;
}

void KusudamaConstraint::set_twist_constraint(Ref<TwistConstraint> p_twist_constraint) {
	ERR_FAIL_COND(p_twist_constraint.is_null());
	set_twist_limits(twist->get_min_twist_angle(), from_tau(twist->get_range()));
	_change_notify();
}

void KusudamaConstraint::snap_to_limits() {
}
