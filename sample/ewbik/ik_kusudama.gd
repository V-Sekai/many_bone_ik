@tool
extends Object

class_name IKKusudama

var limiting_axes: IKNode3D
var twist_axes: IKNode3D
var painfullness: float = 0.5
var cushion_ratio: float = 0.1

var limit_cones: Array = []

var min_axial_angle: float = PI
var range: float = PI * 3

var orientationally_constrained: bool = false
var axially_constrained: bool = false

var strength: float = 1.0

var attached_to: IKBone
var twist_min_rot: Quaternion
var twist_max_rot: Quaternion
var twist_center_rot: Quaternion

func _init(for_bone: IKBone) -> void:
	attached_to = for_bone
	limiting_axes = for_bone.get_major_rotation_axes()
	twist_axes = limiting_axes.attached_copy(false)
	attached_to.add_constraint(self)
	enable()

func constraint_update_notification() -> void:
	update_tangent_radii()
	update_rotational_freedom()

func optimize_limiting_axes() -> void:
	var original_limiting_axes: IKNode3D = twist_axes.get_global_copy()

	var directions: Array = []
	if get_limit_cones().size() == 1:
		directions.append(limit_cones[0].get_control_point().copy())
	else:
		for i in range(get_limit_cones().size() - 1):
			var this_c: Vector3 = get_limit_cones()[i].get_control_point().copy()
			var next_c: Vector3 = get_limit_cones()[i + 1].get_control_point().copy()
			var this_to_next: Quaternion = Quaternion(this_c, next_c)
			var half_this_to_next: Quaternion = Quaternion(this_to_next.get_axis(), this_to_next.get_angle() / 2.0)

			var half_angle: Vector3 = half_this_to_next.apply_to_copy(this_c)
			half_angle = half_angle.normalized()
			half_angle *= this_to_next.get_angle()
			directions.append(half_angle)

	var new_y: Vector3 = Vector3()
	for dv in directions:
		new_y += dv

	new_y /= directions.size()
	if new_y.length() != 0 and not isnan(new_y.y):
		new_y = new_y.normalized()
	else:
		new_y = Vector3(0, 1.0, 0)

	var new_y_ray: IKRay3D = IKRay3D(Vector3(0, 0, 0), new_y)

	var old_y_to_new_y: Quaternion = Quaternion(twist_axes.y_().heading(),
									 original_limiting_axes.get_global_of(new_y_ray).heading())
	twist_axes.rotate_by(old_y_to_new_y)

	for lc in get_limit_cones():
		original_limiting_axes.set_to_global_of(lc.control_point, lc.control_point)
		twist_axes.set_to_local_of(lc.control_point, lc.control_point)
		lc.control_point.normalize()

		update_tangent_radii()

func update_tangent_radii() -> void:
	for i in range(limit_cones.size()):
		var next: IKLimitCone = limit_cones[i + 1] if i < limit_cones.size() - 1 else null
		limit_cones[i].update_tangent_handles(next)

func snap_to_limits() -> void:
	if orientationally_constrained:
		set_axes_to_orientation_snap(attached_to.local_axes(), swing_orientation_axes())
	if axially_constrained:
		snap_to_twist_limits(attached_to.local_axes(), twist_orientation_axes())

func set_axes_to_snapped(to_set: IKNode3D, limiting_axes: IKNode3D, twist_axes: IKNode3D) -> void:
	if limiting_axes != null:
		if orientationally_constrained:
			set_axes_to_orientation_snap(to_set, limiting_axes)
		if axially_constrained:
			snap_to_twist_limits(to_set, twist_axes)

func clamp(value: float, min: float, max: float) -> float:
	if value < min:
		return min
	if value > max:
		return max
	return value

func set_axes_to_returnfulled(to_set: IKNode3D, swing_axes: IKNode3D, twist_axes: IKNode3D,
							   cos_half_returnfullness: float, angle_returnfullness: float) -> void:
	if swing_axes != null and painfullness > 0.0:
		if orientationally_constrained:
			var origin: Vector3 = to_set.origin_()
			var in_point: Vector3 = to_set.y_().p2().copy()
			var path_point: Vector3 = point_on_path_sequence(in_point, swing_axes)
			in_point -= origin
			path_point -= origin
			var to_clamp: Quaternion = Quaternion(in_point, path_point)
			to_clamp.rotation.clamp_to_quadrance_angle(cos_half_returnfullness)
			to_set.rotate_by(to_clamp)
		if axially_constrained:
			var angle_to_twist_mid: float = angle_to_twist_center(to_set, twist_axes)
			var clamped_angle: float = clamp(angle_to_twist_mid, -angle_returnfullness, angle_returnfullness)
			to_set.rotate_about_y(clamped_angle, false)

# A value between between 0 and 1 dictating
# how much the bone to which this kusudama belongs
# prefers to be away from the edges of the kusudama
# if it can. This is useful for avoiding unnatural poses,
# as the kusudama will push bones back into their more
# "comfortable" regions. Leave this value at its default of
# -1 unless your empirical observations show you need it, as its computation
# isn't free.
# Setting this value to anything higher than 0.4 is probably overkill
# in most situations.
#
# @param amount set to a value outside of 0-1 to disable
func set_painfullness(amount: float) -> void:
	painfullness = amount
	if attached_to() != null and attached_to().parent_armature != null:
		attached_to().parent_armature.update_shadow_skel_rate_info()

# @return A value between (ideally between 0 and 1) dictating
#         how much the bone to which this kusudama belongs
#         prefers to be away from the edges of the kusudama
#         if it can.
func get_painfulness() -> float:
	return painfullness

func is_in_limits_(global_point: Vector3) -> bool:
	var in_bounds: Array[float] = [1.0]
	var in_limits: Vector3 = point_in_limits(global_point, in_bounds, IKLimitCone.BOUNDARY)
	return in_bounds[0] > 0.0

# Presumes the input axes are the bone's local_axes, and rotates
# them to satisfy the snap limits.
#
# @param to_set
func set_axes_to_soft_orientation_snap(to_set: IKNode3D, limiting_axes: IKNode3D, cos_half_angle_dampen: float) -> void:
	var in_bounds: Array[float] = [1.0]
	limiting_axes.update_global()
	bone_ray.p1.set(limiting_axes.origin_())
	bone_ray.p2.set(to_set.y_().p2())
	var bonetip: Vector3 = limiting_axes.get_local_of(to_set.y_().p2())
	var in_cushion_limits: Vector3 = point_in_limits(bonetip, in_bounds, IKLimitCone.CUSHION)

	if in_bounds[0] == -1 and in_cushion_limits != null:
		constrained_ray.p1.set(bone_ray.p1())
		constrained_ray.p2.set(limiting_axes.get_global_of(in_cushion_limits))
		var rectified_rot: Quaternion = Quaternion(bone_ray.heading(), constrained_ray.heading())
		to_set.rotate_by(rectified_rot)
		to_set.update_global()

# Presumes the input axes are the bone's local_axes, and rotates
# them to satisfy the snap limits.
#
# @param to_set
func set_axes_to_orientation_snap(to_set: IKNode3D, limiting_axes: IKNode3D) -> void:
	var in_bounds: Array[float] = [1.0]
	limiting_axes.update_global()
	bone_ray.p1.set(limiting_axes.origin_())
	bone_ray.p2.set(to_set.y_().p2())
	var bonetip: Vector3 = limiting_axes.get_local_of(to_set.y_().p2())
	var in_limits: Vector3 = point_in_limits(bonetip, in_bounds, IKLimitCone.BOUNDARY)

	if in_bounds[0] == -1 and in_limits != null:
		constrained_ray.p1.set(bone_ray.p1())
		constrained_ray.p2.set(limiting_axes.get_global_of(in_limits))
		var rectified_rot: Quaternion = Quaternion(bone_ray.heading(), constrained_ray.heading())
		to_set.rotate_by(rectified_rot)
		to_set.update_global()

func is_in_orientation_limits(global_axes: IKNode3D, limiting_axes: IKNode3D) -> bool:
	var in_bounds: Array[float] = [1.0]
	var localized_point: Vector3 = limiting_axes.get_local_of(global_axes.y_().p2()).copy().normalized()
	if limit_cones.size() == 1:
		return limit_cones[0].determine_if_in_bounds(null, localized_point)
	else:
		for i in range(limit_cones.size() - 1):
			if limit_cones[i].determine_if_in_bounds(limit_cones[i + 1], localized_point):
				return true
		return false

var twistMinVec: Vector3
var twistMaxVec: Vector3
var twistTan: Vector3
var twistCenterVec: Vector3
var twistHalfRangeHalfCos: float
var flippedBounds: bool = false

func set_axial_limits(minAngle: float, inRange: float) -> void:
	minAxialAngle = minAngle
	range = inRange
	var y_axis: Vector3 = Vector3(0, 1, 0)
	twistMinRot = Quaternion(y_axis, minAxialAngle)
	twistMinVec = twistMinRot.xform(Vector3(0, 0, 1))
	twistHalfRangeHalfCos = cos(inRange / 4) # for quadrance angle. We need half the range angle since
												# starting from the center, and half of that since quadrance
												# takes cos(angle/2)
	twistMaxVec = Quaternion(y_axis, range).xform(twistMinVec)
	twistMaxRot = Quaternion(y_axis, range)
	var halfRot: Quaternion = Quaternion(y_axis, range / 2)
	twistCenterVec = halfRot.xform(twistMinVec)
	twistCenterRot = Quaternion(Vector3(0, 0, 1), twistCenterVec)
	var maxcross: Vector3 = twistMaxVec.cross(y_axis)
	constraint_update_notification()

func get_twist_ratio() -> float:
	return get_twist_ratio(attached_to().local_axes())

func get_twist_ratio_ik_node(toGet: IKNode3D) -> float:
	return get_twist_ratio(toGet, twistAxes)

func set_twist(ratio: float) -> void:
	set_twist(ratio, attached_to.local_axes())

func set_twist_ik_node(ratio: float, toSet: IKNode3D) -> void:
	var alignRot: Quaternion = twistAxes.get_global_m_basis().inverse_rotation.xform(toSet.get_global_m_basis().rotation)
	var decomposition: Array = alignRot.get_swing_twist(Vector3(0, 1, 0))
	decomposition[1] = Quaternion.slerp(ratio, twistMinRot.rotation, twistMaxRot.rotation)
	var recomposition: Quaternion = decomposition[0].xform(decomposition[1])
	toSet.parent_axes().get_global_m_basis().inverse_rotation.xform(twistAxes.get_global_m_basis().rotation.xform(recomposition), toSet.local_m_basis.rotation)
	toSet.mark_dirty()

func get_twist_ratio_ik_node_twist_axes(toGet: IKNode3D, twistAxes: IKNode3D) -> float:
	var align_rot: Quaternion = twist_axes.get_global_transform().basis.inverse().xform(to_get.get_global_transform().basis).get_rotation_quat()
	var decomposition: Array = align_rot.get_swing_twist(Vector3(0, 1, 0))
	var twist_z: Vector3 = decomposition[1].xform(Vector3(0, 0, 1))
	var min_to_z: Quaternion = Quaternion(twist_min_vec, twist_z)
	var min_to_center: Quaternion = Quaternion(twist_min_vec, twist_center_vec)
	var min_to_z_angle: float = min_to_z.get_angle()
	var min_to_max_angle: float = range
	var flipper: float = 1
	if min_to_center.get_axis().dot(min_to_z.get_axis()) < 0:
		flipper = -1
	return (min_to_z_angle * flipper) / min_to_max_angle

func snap_to_twist_limits(toSet: IKNode3D, twistAxes: IKNode3D) -> float:
	if not axiallyConstrained:
		return 0.0
	var globTwistCent: Quaternion = twistAxes.get_global_m_basis().rotation.xform(twistCenterRot) # create a temporary
																							# orientation representing
																							# globalOf((0,0,1))
																							# represents the middle of
																							# the allowable twist range
																							# in global space
	var alignRot: Quaternion = globTwistCent.inverse().xform(toSet.get_global_m_basis().rotation)
	var decomposition: Array = alignRot.get_swing_twist(Vector3(0, 1, 0)) # decompose the orientation to a swing and
																			# twist away from globTwistCent's
																			# global basis
	decomposition[1].rotation.clamp_to_quadrance_angle(twistHalfRangeHalfCos)
	var recomposition: Quaternion = decomposition[0].xform(decomposition[1])
	toSet.parent_axes().get_global_m_basis().inverse_rotation.xform(globTwistCent.xform(recomposition), toSet.local_m_basis.rotation)
	toSet.local_m_basis.refresh_precomputed()
	toSet.mark_dirty()
	return 0

func angle_to_twist_center(toSet: IKNode3D, twistAxes: IKNode3D) -> float:
	if not axiallyConstrained:
		return 0.0
	var invRot: Quaternion = twistAxes.get_global_m_basis().get_inverse_rotation()
	var alignRot: Quaternion = invRot.xform(toSet.get_global_m_basis().rotation)
	var decomposition: Array = alignRot.get_swing_twist(Vector3(0, 1, 0))
	var twistedDir: Vector3 = decomposition[1].xform(Vector3(0, 0, 1))
	var toMid: Quaternion = Quaternion(twistedDir, twistCenterVec)
	return toMid.get_angle() * toMid.get_axis().y

func in_twist_limits(boneAxes: IKNode3D, limitingAxes: IKNode3D) -> bool:
	var invRot: Quaternion = limitingAxes.get_global_m_basis().get_inverse_rotation()
	var alignRot: Quaternion = invRot.xform(boneAxes.get_global_m_basis().rotation)
	var decomposition: Array = alignRot.get_swing_twist(Vector3(0, 1, 0))
	var angleDelta2: float = decomposition[1].get_angle() * decomposition[1].get_axis().y * -1.0
	angleDelta2 = to_tau(angleDelta2)
	var fromMinToAngleDelta: float = to_tau(signed_angle_difference(angleDelta2, TAU - min_axial_angle()))
	if fromMinToAngleDelta < TAU - range:
		return false
	else:
		return true


func signed_angle_difference(min_angle: float, base: float) -> float:
	var d: float = abs(min_angle - base) % TAU
	var r: float = d
	if d > PI:
		r = TAU - d
	var sign: float = -1.0
	if (min_angle - base >= 0 and min_angle - base <= PI) or (min_angle - base <= -PI and min_angle - base >= -TAU):
		sign = 1.0
	r *= sign
	return r


func point_in_limits(inPoint: Vector3, inBounds: Array, boundaryMode: int) -> Vector3:
	var point: Vector3 = inPoint.normalized()

	inBounds[0] = -1

	var closestCollisionPoint: Vector3 = null
	var closestCos: float = -2.0

	if limitCones.size() > 1 and orientationallyConstrained:
		for i in range(limitCones.size() - 1):
			var collisionPoint: Vector3
			var nextCone: IKLimitCone = limitCones[i + 1]
			var inSegBounds: bool = limitCones[i].in_bounds_from_this_to_next(nextCone, point, collisionPoint)

			if inSegBounds:
				inBounds[0] = 1
			else:
				var thisCos: float = collisionPoint.dot(point)
				if closestCollisionPoint == null or thisCos > closestCos:
					closestCollisionPoint = collisionPoint.duplicate()
					closestCos = thisCos

		if inBounds[0] == -1:
			return closestCollisionPoint
		else:
			return inPoint
	elif orientationallyConstrained:
		if point.dot(limitCones[0].get_control_point()) > limitCones[0].get_radius_cosine():
			inBounds[0] = 1
			return inPoint
		else:
			var axis: Vector3 = limitCones[0].get_control_point().cross(point)
			var toLimit: Quaternion = Quaternion(axis, limitCones[0].get_radius())
			return toLimit.xform(limitCones[0].get_control_point())
	else:
		inBounds[0] = 1

func point_on_path_sequence(in_point: Vector3, limiting_axes: IKNode3D) -> Vector3:
	var closest_point_dot: float = 0.0
	var point: Vector3 = limiting_axes.get_local_of(in_point)
	point.normalize()
	var result: Vector3 = point.duplicate()

	if limit_cones.size() == 1:
		result = limit_cones[0].control_point
	else:
		for i in range(limit_cones.size() - 1):
			var next_cone: IKLimitCone = limit_cones[i + 1]
			var closest_path_point: Vector3 = limit_cones[i].get_closest_path_point(next_cone, point)
			var close_dot: float = closest_path_point.dot(point)
			if close_dot > closest_point_dot:
				result = closest_path_point
				closest_point_dot = close_dot

	return limiting_axes.get_global_of(result)

func get_attached_to() -> IKBone:
	return self.attached_to

func add_limit_cone(new_point: Vector3, radius: float, previous: IKLimitCone, next: IKLimitCone) -> void:
	var insert_at: int = 0

	if next == null or limit_cones.size() == 0:
		add_limit_cone_at_index(-1, new_point, radius)
	elif previous != null:
		insert_at = limit_cones.find(previous) + 1
	else:
		insert_at = max(0, limit_cones.find(next))
	add_limit_cone_at_index(insert_at, new_point, radius)

func remove_limit_cone(limit_cone: IKLimitCone) -> void:
	limit_cones.erase(limit_cone)
	update_tangent_radii()
	update_rotational_freedom()

func add_limit_cone_at_index(insert_at: int, new_point: Vector3, radius: float) -> void:
	var new_cone: IKLimitCone = create_limit_cone_for_index(insert_at, new_point, radius)
	if insert_at == -1:
		limit_cones.append(new_cone)
	else:
		limit_cones.insert(insert_at, new_cone)
	update_tangent_radii()
	update_rotational_freedom()

func to_tau(angle: float) -> float:
	var result: float = angle
	if angle < 0:
		result = (2 * PI) + angle
	result = fmod(result, PI * 2)
	return result

func mod(x: float, y: float) -> float:
	if y != 0 and x != 0:
		var result: float = fmod(x, y)
		if result < 0:
			result += y
		return result
	else:
		return 0
