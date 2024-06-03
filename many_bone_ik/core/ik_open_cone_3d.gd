## IKOpenCone3D class in GDScript

extends Resource

class_name IKOpenCone3D

var control_point : Vector3 = Vector3(0, 1, 0)
var radial_point : Vector3

# Radius stored as cosine to save on the acos call necessary for the angle between.
var radius_cosine : float = 0
var radius : float = 0

var parent_kusudama : WeakRef

var tangent_circle_center_next_1 : Vector3
var tangent_circle_center_next_2 : Vector3
var tangent_circle_radius_next : float = 0
var tangent_circle_radius_next_cos : float = 0

# A triangle where the [1] is the tangent_circle_next_n, and [0] and [2]
# are the points at which the tangent circle intersects this IKLimitCone and the
# next IKLimitCone.
var first_triangle_next : Array = [Vector3(), Vector3(), Vector3()]
var second_triangle_next : Array = [Vector3(), Vector3(), Vector3()]

## Placeholder for set_attached_to method
func set_attached_to(p_attached_to: RefCounted) -> void:
	parent_kusudama.set_ref(p_attached_to);

## Placeholder for get_attached_to method
func get_attached_to() -> RefCounted:
	return parent_kusudama.get_ref();


## Placeholder for update_tangent_handles method
func update_tangent_handles(p_next: IKOpenCone3D) -> void:
	if p_next.is_valid():
		var rad_a = get_radius()
		var rad_b = p_next.get_radius()

		var a = get_control_point()
		var b = p_next.get_control_point()

		var arc_normal = a.cross(b).normalized()

		var t_radius = (PI - (rad_a + rad_b)) / 2

		var boundary_plus_tangent_radius_a = rad_a + t_radius
		var boundary_plus_tangent_radius_b = rad_b + t_radius

		var scaled_axis_a = a * cos(boundary_plus_tangent_radius_a)
		var temp_var = IKKusudama3D.get_quaternion_axis_angle(arc_normal, boundary_plus_tangent_radius_a)
		var plane_dir1_a = temp_var.xform(a)
		var temp_var2 = IKKusudama3D.get_quaternion_axis_angle(a, PI / 2)
		var plane_dir2_a = temp_var2.xform(plane_dir1_a)

		var scaled_axis_b = b * cos(boundary_plus_tangent_radius_b)
		var temp_var3 = IKKusudama3D.get_quaternion_axis_angle(arc_normal, boundary_plus_tangent_radius_b)
		var plane_dir1_b = temp_var3.xform(b)
		var temp_var4 = IKKusudama3D.get_quaternion_axis_angle(b, PI / 2)
		var plane_dir2_b = temp_var4.xform(plane_dir1_b)

		var r1_b = IKRay3D.new(plane_dir1_b, scaled_axis_b)
		var r2_b = IKRay3D.new(plane_dir1_b, plane_dir2_b)

		r1_b.elongate(99)
		r2_b.elongate(99)

		var intersection1 = r1_b.get_intersects_plane(scaled_axis_a, plane_dir1_a, plane_dir2_a)
		var intersection2 = r2_b.get_intersects_plane(scaled_axis_a, plane_dir1_a, plane_dir2_a)

		var intersection_ray = IKRay3D.new(intersection1, intersection2)
		intersection_ray.elongate(99)

		var sphere_intersect1
		var sphere_intersect2
		var sphere_center
		intersection_ray.intersects_sphere(sphere_center, 1.0, sphere_intersect1, sphere_intersect2)

		set_tangent_circle_center_next_1(sphere_intersect1)
		set_tangent_circle_center_next_2(sphere_intersect2)
		_set_tangent_circle_radius_next(t_radius)

	if tangent_circle_center_next_1 == Vector3(NAN, NAN, NAN):
		tangent_circle_center_next_1 = _get_orthogonal(control_point).normalized()

	if tangent_circle_center_next_2 == Vector3(NAN, NAN, NAN):
		tangent_circle_center_next_2 = (tangent_circle_center_next_1 * -1).normalized()

	if p_next.is_valid():
		compute_triangles(p_next)


## Placeholder for set_tangent_circle_center_next_1 method
func set_tangent_circle_center_next_1(point: Vector3) -> void:
	tangent_circle_center_next_1 = point.normalized()

# Placeholder for set_tangent_circle_center_next_2 method
func set_tangent_circle_center_next_2(point: Vector3) -> void:
	tangent_circle_center_next_2 = point.normalized()


## Placeholder for get_on_great_tangent_triangle method
func get_on_great_tangent_triangle(next: IKOpenCone3D, input: Vector3) -> Vector3:
	assert(next != null)

	var c1xc2 = control_point.cross(next.control_point)
	var c1c2dir = input.dot(c1xc2)

	if c1c2dir < 0.0:
		var c1xt1 = control_point.cross(tangent_circle_center_next_1).normalized()
		var t1xc2 = tangent_circle_center_next_1.cross(next.control_point).normalized()

		if input.dot(c1xt1) > 0 and input.dot(t1xc2) > 0:
			var to_next_cos = input.dot(tangent_circle_center_next_1)

			if to_next_cos > tangent_circle_radius_next_cos:
				var plane_normal = tangent_circle_center_next_1.cross(input).normalized()
				plane_normal.normalize()
				var rotate_about_by = Quaternion(plane_normal, tangent_circle_radius_next)
				return rotate_about_by.xform(tangent_circle_center_next_1)
			else:
				return input
		else:
			return Vector3(NAN, NAN, NAN)
	else:
		var t2xc1 = tangent_circle_center_next_2.cross(control_point).normalized()
		var c2xt2 = next.control_point.cross(tangent_circle_center_next_2).normalized()

		if input.dot(t2xc1) > 0 and input.dot(c2xt2) > 0:
			if input.dot(tangent_circle_center_next_2) > tangent_circle_radius_next_cos:
				var plane_normal = tangent_circle_center_next_2.cross(input).normalized()
				plane_normal.normalize()
				var rotate_about_by = Quaternion(plane_normal, tangent_circle_radius_next)
				return rotate_about_by.xform(tangent_circle_center_next_2)
			else:
				return input
		else:
			return Vector3(NAN, NAN, NAN)


## Placeholder for get_tangent_circle_radius_next method
func get_tangent_circle_radius_next() -> float:
	return tangent_circle_radius_next

## Placeholder for get_tangent_circle_center_next_1 method
func get_tangent_circle_center_next_1() -> Vector3:
	return tangent_circle_center_next_1

## Placeholder for get_tangent_circle_center_next_2 method
func get_tangent_circle_center_next_2() -> Vector3:
	return tangent_circle_center_next_2

func closest_to_cone(input: Vector3, in_bounds: Array) -> Vector3:
	var normalized_input = input.normalized()
	var normalized_control_point = get_control_point().normalized()

	if normalized_input.dot(normalized_control_point) > get_radius_cosine():
		if in_bounds != null:
			in_bounds[0] = 1.0
		return Vector3(NAN, NAN, NAN)

	var axis = normalized_control_point.cross(normalized_input).normalized()

	if is_zero_approx(axis.length_squared()) or !axis.is_finite():
		axis = Vector3(0, 1, 0)

	var rot_to = IKKusudama3D.get_quaternion_axis_angle(axis, get_radius())
	var axis_control_point = normalized_control_point

	if is_zero_approx(axis_control_point.length_squared()):
		axis_control_point = Vector3(0, 1, 0)

	var result = rot_to.xform(axis_control_point)

	if in_bounds != null:
		in_bounds[0] = -1

	return result


## Placeholder for get_closest_path_point method
func get_closest_path_point(next: IKOpenCone3D, input: Vector3) -> Vector3:
	var result = Vector3()
	if next == null:
		result = _closest_cone(self, input)
	else:
		result = _get_on_path_sequence(next, input)
		var is_number = !(is_nan(result.x) and is_nan(result.y) and is_nan(result.z))

		if not is_number:
			result = _closest_cone(next, input)
	return result


## Placeholder for get_control_point method
func get_control_point() -> Vector3:
	return control_point


## Placeholder for set_control_point method
func set_control_point(p_control_point: Vector3) -> void:
	if p_control_point.length_squared() == 0:
		self.control_point = Vector3(0, 1, 0)
	else:
		self.control_point = p_control_point
		self.control_point.normalize()



## Placeholder for get_radius method
func get_radius() -> float:
	return radius;

## Placeholder for get_radius_cosine method
func get_radius_cosine() -> float:
	return radius_cosine

## Placeholder for set_radius method
func set_radius(radius: float) -> void:
	radius = p_radius;
	radius_cosine = cos(p_radius);

# Private

## Placeholder for _compute_triangles method
func _compute_triangles(p_next: IKOpenCone3D) -> void:
	## If the next cone is null, exit the function
	if p_next == null:
		return

	## Compute the normalized vectors for the first triangle
	first_triangle_next[1] = tangent_circle_center_next_1.normalized()
	first_triangle_next[0] = get_control_point().normalized()
	first_triangle_next[2] = p_next.get_control_point().normalized()

	## Compute the normalized vectors for the second triangle
	second_triangle_next[1] = tangent_circle_center_next_2.normalized()
	second_triangle_next[0] = get_control_point().normalized()
	second_triangle_next[2] = p_next.get_control_point().normalized()


func _closest_cone(next: IKOpenCone3D, input: Vector3) -> Vector3:
	if next == null:
		return control_point

	if input.dot(control_point) > input.dot(next.control_point):
		return control_point
	else:
		return next.control_point


# Placeholder for _set_tangent_circle_radius_next method
func _set_tangent_circle_radius_next(radius) -> void:
	tangent_circle_radius_next = radius;
	tangent_circle_radius_next_cos = cos(tangent_circle_radius_next);


func _get_closest_collision(next: IKOpenCone3D, input: Vector3) -> Vector3:
	if next == null:
		return input

	var result = Vector3()

	if next == null:
		var in_bounds = [0.0]
		result = _closest_cone(null, input)
	else:
		result = get_on_great_tangent_triangle(next, input)
		var is_number = !(is_nan(result.x) and is_nan(result.y) and is_nan(result.z))

		if not is_number:
			var in_bounds = [0.0]
			result = _closest_point_on_closest_cone(next, input, in_bounds)
	
	return result


## Placeholder for _determine_if_in_bounds method
func _determine_if_in_bounds(next: IKOpenCone3D, input: Vector3) -> bool:
	## Procedure : Check if input is contained in this cone, or the next cone
	## if it is, then we're finished and in bounds. otherwise,
	## check if the point  is contained within the tangent radii,
	## if it is, then we're out of bounds and finished, otherwise
	## in the tangent triangles while still remaining outside of the tangent radii
	## if it is, then we're finished and in bounds. otherwise, we're out of bounds.

	if control_point.dot(input) >= radius_cosine:
		return true
	elif next != null and next.control_point.dot(input) >= next.radius_cosine:
		return true
	else:
		if next == null:
			return false
		var in_tan1_rad = tangent_circle_center_next_1.dot(input) > tangent_circle_radius_next_cos
		if in_tan1_rad:
			return false
		var in_tan2_rad = tangent_circle_center_next_2.dot(input) > tangent_circle_radius_next_cos
		if in_tan2_rad:
			return false

		## if we reach this point in the code, we are either on the path between two open_cones, or on the path extending out from between them
		## but outside of their radii.
		## To determine which , we take the cross product of each control point with each tangent center.
		## The direction of each of the resultant vectors will represent the normal of a plane.
		## Each of these four planes define part of a boundary which determines if our point is in bounds.
		## If the dot product of our point with the normal of any of these planes is negative, we must be out
		## of bounds.
		##
		## Older version of this code relied on a triangle intersection algorithm here, which I think is slightly less efficient on average
		## as it didn't allow for early termination. .

		var c1xc2 = control_point.cross(next.control_point)
		var c1c2dir = input.dot(c1xc2)

		if c1c2dir < 0.0:
			var c1xt1 = control_point.cross(tangent_circle_center_next_1)
			var t1xc2 = tangent_circle_center_next_1.cross(next.control_point)
			return input.dot(c1xt1) > 0 and input.dot(t1xc2) > 0
		else:
			var t2xc1 = tangent_circle_center_next_2.cross(control_point)
			var c2xt2 = next.control_point.cross(tangent_circle_center_next_2)
			return input.dot(t2xc1) > 0 and input.dot(c2xt2) > 0


# Placeholder for _get_on_path_sequence method
func _get_on_path_sequence(next: IKOpenCone3D, input: Vector3) -> Vector3:
	if next == null:
		return Vector3(NAN, NAN, NAN)
	
	var c1xc2 = get_control_point().cross(next.control_point).normalized()
	var c1c2dir = input.dot(c1xc2)
	
	if c1c2dir < 0.0:
		var c1xt1 = get_control_point().cross(tangent_circle_center_next_1).normalized()
		var t1xc2 = tangent_circle_center_next_1.cross(next.get_control_point()).normalized()
		
		if input.dot(c1xt1) > 0.0 and input.dot(t1xc2) > 0.0:
			var tan1ToInput = IKRay3D.new(tangent_circle_center_next_1, input)
			var result = tan1ToInput.get_intersects_plane(Vector3(0.0, 0.0, 0.0), get_control_point(), next.get_control_point())
			return result.normalized()
		else:
			return Vector3(NAN, NAN, NAN)
	else:
		var t2xc1 = tangent_circle_center_next_2.cross(control_point).normalized()
		var c2xt2 = next.get_control_point().cross(tangent_circle_center_next_2).normalized()
		
		if input.dot(t2xc1) > 0 and input.dot(c2xt2) > 0:
			var tan2ToInput = IKRay3D.new(tangent_circle_center_next_2, input)
			var result = tan2ToInput.get_intersects_plane(Vector3(0.0, 0.0, 0.0), get_control_point(), next.get_control_point())
			return result.normalized()
		else:
			return Vector3(NAN, NAN, NAN)


func _closest_point_on_closest_cone(next: IKOpenCone3D, input: Vector3, in_bounds: Array) -> Vector3:
	if next == null:
		return input
	var closestToFirst = closest_to_cone(input, in_bounds)
	if in_bounds != null and in_bounds[0] > 0.0:
		return closestToFirst
	if next == null:
		return closestToFirst
	else:
		var closestToSecond = next.closest_to_cone(input, in_bounds)

		if in_bounds != null and in_bounds[0] > 0.0:
			return closestToSecond

		var cosToFirst = input.dot(closestToFirst)
		var cosToSecond = input.dot(closestToSecond)

		if cosToFirst > cosToSecond:
			return closestToFirst
		else:
			return closestToSecond


# Placeholder for _get_tangent_circle_radius_next_cos method
func _get_tangent_circle_radius_next_cos() -> float:
	return tangent_circle_radius_next_cos

func _get_orthogonal(p_in: Vector3) -> Vector3:
	var result = Vector3()
	var threshold = p_in.length() * 0.6

	if threshold > 0.0:
		if abs(p_in.x) <= threshold:
			var inverse = 1.0 / sqrt(p_in.y * p_in.y + p_in.z * p_in.z)
			return Vector3(0.0, inverse * p_in.z, -inverse * p_in.y)
		elif abs(p_in.y) <= threshold:
			var inverse = 1.0 / sqrt(p_in.x * p_in.x + p_in.z * p_in.z)
			return Vector3(-inverse * p_in.z, 0.0, inverse * p_in.x)

		var inverse = 1.0 / sqrt(p_in.x * p_in.x + p_in.y * p_in.y)
		return Vector3(inverse * p_in.y, -inverse * p_in.x, 0.0)

	return result


# Placeholder for _get_radius method
func _get_radius() -> float:
	return radius
