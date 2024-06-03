extends Resource
class_name IKKusudama3D

## An array containing all of the Kusudama's open_cones. The kusudama is built up
## with the expectation that any limitCone in the array is connected to the cone at the previous element in the array,
## and the cone at the next element in the array.
var open_cones : Array = []

var twist_min_rot : Quaternion
var twist_min_vec : Vector3
var twist_max_vec : Vector3
var twist_center_vec : Vector3
var twist_center_rot : Quaternion
var twist_max_rot : Quaternion
var twist_half_range_half_cos : float = 0
var twist_tan : Vector3
var flipped_bounds : bool = false
var resistance : float = 0

## Defined as some Angle in radians about the limiting_axes Y axis, 0 being equivalent to the
## limiting_axes Z axis.
var min_axial_angle : float = 0.0
## Defined as some Angle in radians about the limiting_axes Y axis, 0 being equivalent to the
## min_axial_angle
var range_angle : float = TAU

var orientationally_constrained : bool = false
var axially_constrained : bool = false

var bone_ray : IKRay3D = IKRay3D.new()
var constrained_ray : IKRay3D = IKRay3D.new()
var unit_hyper_area : float = 2 * pow(PI, 2)
var unit_area : float = 4 * PI

func _update_constraint(p_limiting_axes: Node) -> void:
	var directions = []
	
	if open_cones.size() == 1 and open_cones[0] != null:
		directions.append(open_cones[0].get_control_point())
	else:
		for i in range(open_cones.size() - 1):
			if open_cones[i] == null or open_cones[i + 1] == null:
				continue
			
			var this_control_point = open_cones[i].get_control_point()
			var next_control_point = open_cones[i + 1].get_control_point()
			
			var this_to_next = Quaternion(this_control_point, next_control_point)
			
			var axis = this_to_next.get_axis()
			var angle = this_to_next.get_angle() / 2.0
			
			var half_angle = this_control_point.rotated(axis, angle)
			half_angle *= this_to_next.get_angle()
			half_angle.normalize()
			
			directions.append(half_angle)
	
	var new_y = Vector3()
	for direction_vector in directions:
		new_y += direction_vector
	
	if directions.size() > 0:
		new_y /= directions.size()
		new_y.normalize()
	
	var new_y_ray = Transform3D(Basis(), new_y)
	var old_y_to_new_y = Quaternion(p_limiting_axes.get_global_transform().basis.get_column(Vector3.AXIS_Y).normalized(), p_limiting_axes.get_global_transform().basis.xform(new_y_ray.origin).normalized())
	
	p_limiting_axes.rotate_local_with_global(old_y_to_new_y)
	
	for open_cone in open_cones:
		if open_cone == null:
			continue
		
		var control_point = open_cone.get_control_point()
		open_cone.set_control_point(control_point.normalized())
	
	update_tangent_radii()


func update_tangent_radii() -> void:
	for i in range(open_cones.size()):
		var current = open_cones[i]
		var next = null
		if i < open_cones.size() - 1:
			next = open_cones[i + 1]
		
		var cone = open_cones[i]
		cone.update_tangent_handles(next)


## Get the swing rotation and twist rotation for the specified axis. The twist rotation represents the rotation around the specified axis. The swing rotation represents the rotation of the specified
## axis itself, which is the rotation around an axis perpendicular to the specified axis. The swing and twist rotation can be
## used to reconstruct the original quaternion: this = swing * twist
##
## @param p_axis the X, Y, Z component of the normalized axis for which to get the swing and twist rotation
## @return twist represent the rotational twist
## @return swing represent the rotational swing
## @see <a href="http://www.euclideanspace.com/maths/geometry/rotations/for/decomposition">calculation</a>
static func get_swing_twist(p_rotation: Quaternion, p_axis: Vector3) -> Dictionary:
	assert(p_rotation.is_normalized(), "The quaternion must be normalized.")

	if p_axis.length_squared() == 0:
		return {"r_swing": Quaternion(), "r_twist": Quaternion()}

	var rotation = p_rotation
	if rotation.w < 0.0:
		rotation *= -1

	var p = p_axis * (rotation.x * p_axis.x + rotation.y * p_axis.y + rotation.z * p_axis.z)
	var r_twist = Quaternion(p.x, p.y, p.z, rotation.w).normalized()
	var d = Vector3(r_twist.x, r_twist.y, r_twist.z).dot(p_axis)

	if d < 0.0:
		r_twist *= -1.0

	var r_swing = (rotation * r_twist.inverse()).normalized()

	return {"r_swing": r_swing, "r_twist": r_twist}


static func get_quaternion_axis_angle(p_axis: Vector3, p_angle: float) -> Quaternion:
	var d = p_axis.length_squared()
	if d == 0:
		return Quaternion()
	else:
		var sin_angle = sin(p_angle * 0.5)
		var cos_angle = cos(p_angle * 0.5)
		var s = sin_angle / d
		return Quaternion(p_axis.x * s, p_axis.y * s, p_axis.z * s, cos_angle)

#
### Presumes the input axes are the bone's localAxes, and rotates
### them to satisfy the snap limits.
###
### @param to_set
#func snap_to_orientation_limit(p_bone_direction: Node, p_to_set: Node, p_limiting_axes: Node, p_dampening: float, p_cos_half_angle_dampen: float) -> void:
	#if p_bone_direction == null or p_to_set == null or p_limiting_axes == null:
		#return
#
	#var in_bounds = [1.0]
	#var limiting_origin = p_limiting_axes.global_transform.origin
	#var bone_dir_xform = p_bone_direction.global_transform.xform(Vector3(0.0, 1.0, 0.0))
#
	#var bone_ray = RayCast.new()
	#bone_ray.set_point_1(limiting_origin)
	#bone_ray.set_point_2(bone_dir_xform)
#
	#var bone_tip = p_limiting_axes.to_local(bone_ray.get_point_2())
	#var in_limits = get_local_point_in_limits(bone_tip, in_bounds)
#
	#if in_bounds[0] < 0:
		#var constrained_ray = RayCast.new()
		#constrained_ray.set_point_1(bone_ray.get_point_1())
		#constrained_ray.set_point_2(p_limiting_axes.to_global(in_limits))
#
		#var rectified_rot = Quaternion(bone_ray.get_heading(), constrained_ray.get_heading())
		#p_to_set.rotate_local_with_global(rectified_rot)


## Kusudama constraints decompose the bone orientation into a swing component, and a twist component.
## The "Swing" component is the final direction of the bone. The "Twist" component represents how much
## the bone is rotated about its own final direction. Where limit cones allow you to constrain the "Swing"
## component, this method lets you constrain the "twist" component.
##
## @param min_angle some angle in radians about the major rotation frame's y-axis to serve as the first angle within the range_angle that the bone is allowed to twist.
## @param in_range some angle in radians added to the min_angle. if the bone's local Z goes maxAngle radians beyond the min_angle, it is considered past the limit.
## This value is always interpreted as being in the positive direction. For example, if this value is -PI/2, the entire range_angle from min_angle to min_angle + 3PI/4 is
## considered valid.
func set_axial_limits(p_min_angle: float, p_in_range: float) -> void:
	min_axial_angle = p_min_angle
	range_angle = p_in_range
	var y_axis = Vector3(0.0, 1.0, 0.0)
	var z_axis = Vector3(0.0, 0.0, 1.0)
	twist_min_rot = get_quaternion_axis_angle(y_axis, min_axial_angle)
	twist_min_vec = (twist_min_rot * z_axis).normalized()
	twist_center_vec = (twist_min_rot * twist_min_vec).normalized()
	twist_center_rot = Quaternion(z_axis, twist_center_vec)
	twist_half_range_half_cos = cos(p_in_range / 4.0) # For the quadrance angle. We need half the range angle since starting from the center, and half of that since quadrance takes cos(angle/2).
	twist_max_vec = get_quaternion_axis_angle(y_axis, p_in_range) * (twist_min_vec).normalized()
	twist_max_rot = Quaternion(z_axis, twist_max_vec)


### @param to_set
### @param limiting_axes
### @return radians of the twist required to snap bone into twist limits (0 if bone is already in twist limits)
#func set_snap_to_twist_limit(p_bone_direction: RefCounted, p_to_set: RefCounted, p_limiting_axes: RefCounted, p_dampening: float, p_cos_half_dampen: float) -> void:
	#if not is_axially_constrained():
		#return
#
	#var global_transform_constraint = p_limiting_axes.get_global_transform()
	#var global_transform_to_set = p_to_set.get_global_transform()
	#var parent_global_inverse = p_to_set.get_parent().get_global_transform().basis.inverse()
	#var global_twist_center = global_transform_constraint.basis * twist_center_rot
	#var align_rot = (global_twist_center.inverse() * global_transform_to_set.basis).orthonormalized()
#
	#var twist_rotation : Quaternion
	#var swing_rotation : Quaternion
#
	#get_swing_twist(align_rot.get_rotation_quat(), Vector3(0, 1, 0), swing_rotation, twist_rotation)
#
	#twist_rotation = IKBoneSegment3D.clamp_to_cos_half_angle(twist_rotation, twist_half_range_half_cos)
#
	#var recomposition = (global_twist_center * (swing_rotation * twist_rotation)).orthonormalized()
	#var rotation = parent_global_inverse * recomposition
#
	#p_to_set.set_transform(Transform(rotation, p_to_set.get_transform().origin))


## Given a point (in local coordinates), checks to see if a ray can be extended from the Kusudama's
## origin to that point, such that the ray in the Kusudama's reference frame is within the range_angle allowed by the Kusudama's
## coneLimits.
## If such a ray exists, the original point is returned (the point is within the limits).
## If it cannot exist, the tip of the ray within the kusudama's limits that would require the least rotation
## to arrive at the input point is returned.
## @param in_point the point to test.
## @param in_bounds should be an array with at least 2 elements. The first element will be set to  a number from -1 to 1 representing the point's distance from the boundary, 0 means the point is right on
## the boundary, 1 means the point is within the boundary and on the path furthest from the boundary. any negative number means
## the point is outside of the boundary, but does not signify anything about how far from the boundary the point is.
## The second element will be given a value corresponding to the limit cone whose bounds were exceeded. If the bounds were exceeded on a segment between two limit cones,
## this value will be set to a non-integer value between the two indices of the limitcone comprising the segment whose bounds were exceeded.
## @return the original point, if it's in limits, or the closest point which is in limits.
func get_local_point_in_limits(in_point: Vector3, in_bounds: Array) -> Vector3:
	var point = in_point.normalized()
	var closest_cos = -2.0
	in_bounds[0] = -1
	var closest_collision_point = in_point

	for i in range(len(open_cones)):
		var cone = open_cones[i]
		var collision_point = cone.closest_to_cone(point, in_bounds)

		if is_nan(collision_point.x) or is_nan(collision_point.y) or is_nan(collision_point.z):
			in_bounds[0] = 1
			return point

		var this_cos = collision_point.dot(point)

		if closest_collision_point.is_zero_approx() or this_cos > closest_cos:
			closest_collision_point = collision_point
			closest_cos = this_cos

	if in_bounds[0] == -1:
		for i in range(len(open_cones) - 1):
			var currCone = open_cones[i]
			var nextCone = open_cones[i + 1]
			var collision_point = currCone.get_on_great_tangent_triangle(nextCone, point)

			if is_nan(collision_point.x):
				continue

			var this_cos = collision_point.dot(point)

			if is_equal_approx(this_cos, 1.0):
				in_bounds[0] = 1
				return point

			if this_cos > closest_cos:
				closest_collision_point = collision_point
				closest_cos = this_cos

	return closest_collision_point




## Add a IKLimitCone to the Kusudama.
## @param new_point where on the Kusudama to add the LimitCone (in Kusudama's local coordinate frame defined by its bone's majorRotationAxes))
## @param radius the radius of the limitCone
func add_open_cone(p_open_cone: Node) -> void:
	assert(p_open_cone != null, "Open cone cannot be null.")
	assert(p_open_cone.get_attached_to() != null, "Attached node cannot be null.")
	assert(p_open_cone.get_tangent_circle_center_next_1().length_squared() != 0, "Tangent circle center next 1 cannot have zero length.")
	assert(p_open_cone.get_tangent_circle_center_next_2().length_squared() != 0, "Tangent circle center next 2 cannot have zero length.")
	assert(p_open_cone.get_control_point().length_squared() != 0, "Control point cannot have zero length.")

	open_cones.append(p_open_cone)


## @return the lower bound on the axial constraint
func get_min_axial_angle() -> float:
	return min_axial_angle

func get_range_angle() -> float:
	return range_angle

func is_axially_constrained() -> bool:
	return axially_constrained

func is_orientationally_constrained() -> bool:
	return orientationally_constrained


static func clamp_to_quadrance_angle(p_rotation: Quaternion, p_cos_half_angle: float) -> Quaternion:
	assert(p_rotation.is_normalized(), "The quaternion must be normalized.")

	var rotation = p_rotation
	var newCoeff = 1.0 - (p_cos_half_angle * abs(p_cos_half_angle))
	var currentCoeff = rotation.x * rotation.x + rotation.y * rotation.y + rotation.z * rotation.z

	if newCoeff >= currentCoeff:
		return rotation

	var over_limit = (currentCoeff - newCoeff) / (1.0 - newCoeff)
	var clamped_rotation = rotation
	if rotation.w < 0:
		clamped_rotation.w = -p_cos_half_angle
	else:
		clamped_rotation.w = p_cos_half_angle

	var compositeCoeff = sqrt(newCoeff / currentCoeff)

	clamped_rotation.x *= compositeCoeff
	clamped_rotation.y *= compositeCoeff
	clamped_rotation.z *= compositeCoeff

	if not rotation.is_finite() or not clamped_rotation.is_finite():
		return Quaternion()

	return rotation.slerp(clamped_rotation, over_limit)

func is_nan_vector(vec: Vector3) -> bool:
	return is_nan(vec.x) or is_nan(vec.y) or is_nan(vec.z)

func local_point_on_path_sequence(in_point: Vector3, limiting_axes: Node) -> Vector3:
	var closest_point_dot = 0.0
	var point = limiting_axes.global_transform.xform(in_point).normalized()
	var result = point

	if open_cones.size() == 1:
		var cone = open_cones[0]
		result = cone.get_control_point()
	else:
		for i in range(open_cones.size() - 1):
			var next_cone = open_cones[i + 1]
			var cone = open_cones[i]
			var closest_path_point = cone.get_closest_path_point(next_cone, point)
			var close_dot = closest_path_point.dot(point)

			if close_dot > closest_point_dot:
				result = closest_path_point
				closest_point_dot = close_dot

	return result


func remove_open_cone(limitCone: Node) -> void:
	if limitCone in open_cones:
		open_cones.erase(limitCone)

func disable_orientational_limits() -> void:
	orientationally_constrained = false

func enable_orientational_limits() -> void:
	orientationally_constrained = true

func toggle_orientational_limits() -> void:
	orientationally_constrained = not orientationally_constrained

func disable_axial_limits() -> void:
	axially_constrained = false

func enable_axial_limits() -> void:
	axially_constrained = true

func toggle_axial_limits() -> void:
	axially_constrained = not axially_constrained

func is_enabled() -> bool:
	return axially_constrained or orientationally_constrained

func disable() -> void:
	axially_constrained = false
	orientationally_constrained = false

func enable() -> void:
	axially_constrained = true
	orientationally_constrained = true

func clear_open_cones() -> void:
	open_cones.clear()

func get_open_cones() -> Array:
	return open_cones

func set_open_cones(p_cones: Array) -> void:
	open_cones = p_cones

func get_resistance() -> float:
	return resistance

func set_resistance(p_resistance: float) -> void:
	resistance = p_resistance
