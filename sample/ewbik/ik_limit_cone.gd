@tool
extends Object

class_name IKLimitCone

var control_point: Vector3

# radius stored as cosine to save on the acos call necessary for angle_between.
var radius_cosine: float
var radius: float
var cushion_radius: float
var cushion_cosine: float
## a value of 0 means the cushion radius will be 0,
## a value of 1 means the cushion radius will be equal to the cone radius
var cushion_ratio: float = 0.8

var parent_kusudama: IKKusudama

var tangent_circle_center_next_1: Vector3
var tangent_circle_center_next_2: Vector3
var tangent_circle_radius_next: float
var tangent_circle_radius_next_cos: float

var cushion_tangent_circle_center_next_1: Vector3
var cushion_tangent_circle_center_next2: Vector3
var cushion_tangent_circle_center_previous1: Vector3
var cushion_tangent_circle_center_previous2: Vector3
var cushion_tangent_circle_radius_next: float
var cushion_tangent_circle_radius_next_cos: float

const BOUNDARY = 0
const CUSHION = 1

## softness of 0 means completely hard.
## any softness higher than 0f means that
## as the softness value is increased
## the is more penalized for moving
## further from the center of the channel
var softness: float = 0.0

## a triangle where the [1] is th tangent_circle_next_n, and [0] and [2]
## are the points at which the tangent circle intersects this limit_cone and the
## next limit_cone
var first_triangle_next: Array = [Vector3(), Vector3(), Vector3()]
var second_triangle_next: Array = [Vector3(), Vector3(), Vector3()]

func set_control_point(control_point: Vector3) -> void:
	self.control_point = control_point.normalized()
	if parent_kusudama != null:
		parent_kusudama.constraint_update_notification()

func get_vector3_orthogonal(vector: Vector3):
	var threshold: float = vector.length() * 0.6
	if threshold > 0:
		if abs(vector.x) <= threshold:
			var inverse: float = 1 / sqrt(vector.y * vector.y + vector.z * vector.z)
			return Vector3(0, inverse * vector.z, -inverse * vector.y)
		elif abs(vector.y) <= threshold:
			var inverse: float = 1 / sqrt(vector.x * vector.x + vector.z * vector.z)
			return Vector3(-inverse * vector.z, 0, inverse * vector.x)
		var inverse: float = 1 / sqrt(vector.x * vector.x + vector.y * vector.y)
		return Vector3(inverse * vector.y, -inverse * vector.x, 0)

#	/**
#	 * 
#	 * @param direction
#	 * @param rad
#	 * @param cushion    range 0-1, how far toward the boundary to begin slowing
#	 *                   down the rotation if soft constraints are enabled.
#	 *                   Value of 1 creates a hard boundary. Value of 0 means it
#	 *                   will always be the case that the closer a joint in the
#	 *                   allowable region
#	 *                   is to the boundary, the more any further rotation in the
#	 *                   direction of that boundary will be avoided.
#	 * @param attachedTo
#	 */
func _init(direction: Vector3, rad: float, cushion: float, attached_to: IKKusudama):
	set_control_point(direction)
	self.tangent_circle_center_next_1 = get_vector3_orthogonal(direction)
	self.tangent_circle_center_next_2 = -tangent_circle_center_next_1
	
	const FLT_MIN = 1.17e-38
	self.radius = max(FLT_MIN, rad)
	self.radius_cosine = cos(radius)
	self.cushion_ratio = min(1, abs(cushion))
	self.cushion_radius = self.radius * self.cushion_ratio
	self.cushion_cosine = cos(cushion_radius)
	self.parent_kusudama = attached_to

#	/**
#	 * 
#	 * @param next
#	 * @param input
#	 * @param collisionPoint will be set to the rectified (if necessary) position of
#	 *                       the input after accounting for collisions
#	 * @return
#	 */
func in_bounds_from_this_to_next(next, input: Vector3, collision_point: Vector3) -> bool:
	var is_in_bounds = false
	var closest_collision = get_closest_collision(next, input)
	if closest_collision == null:
		is_in_bounds = true
		collision_point.x = input.x
		collision_point.y = input.y
		collision_point.z = input.z
	else:
		collision_point.x = closest_collision.x
		collision_point.y = closest_collision.y
		collision_point.z = closest_collision.z
	return is_in_bounds

#	/**
#	 * 
#	 * @param next
#	 * @param input
#	 * @return null if the input point is already in bounds, or the point's
#	 *         rectified position
#	 *         if the point was out of bounds.
#	 */
func get_closest_collision(next, input: Vector3) -> Vector3:
	var result = get_on_great_tangent_triangle(next, input)
	if result == null:
		var in_bounds = [false]
		result = closest_point_on_closest_cone(next, input, in_bounds)
	return result

func get_closest_path_point(next, input: Vector3) -> Vector3:
	var result = get_on_path_sequence(next, input)
	if result == null:
		result = closest_cone(next, input)
	return result


#	/**
#	 * Determines if a ray emanating from the origin to given point in local space
#	 * lies within the path from this cone to the next cone. This function relies on
#	 * an optimization trick for a performance boost, but the trick ruins everything
#	 * if the input isn't normalized. So it is ABSOLUTELY VITAL
#	 * that @param input have unit length in order for this function to work
#	 * correctly.
#	 *
#	 * @param next
#	 * @param input
#	 * @return
#	 */
func determine_if_in_bounds(next, input: Vector3) -> bool:
	
#		/**
#		 * Procedure : Check if input is contained in this cone, or the next cone
#		 * if it is, then we're finished and in bounds. otherwise,
#		 * check if the point is contained within the tangent radii,
#		 * if it is, then we're out of bounds and finished, otherwise
#		 * in the tangent triangles while still remaining outside of the tangent radii
#		 * if it is, then we're finished and in bounds. otherwise, we're out of bounds.
#		 */
	if control_point.dot(input) >= radius_cosine:
		return true
	elif next != null and next.control_point.dot(input) >= next.radius_cosine:
		return true
	else:
		## if we reach this point in the code, we are either on the path between two
		## limitCones, or on the path extending out from between them
		## but outside of their radii.
		## To determine which , we take the cross product of each control point with
		## each tangent center.
		## The direction of each of the resultant vectors will represent the normal of a
		## plane.
		## Each of these four planes define part of a boundary which determines if our
		## point is in bounds.
		## If the dot product of our point with the normal of any of these planes is
		## negative, we must be out
		## of bounds.

		if next == null:
			return false
		var in_tan1_rad = tangent_circle_center_next_1.dot(input) > tangent_circle_radius_next_cos
		if in_tan1_rad:
			return false
		var in_tan2_rad = tangent_circle_center_next_2.dot(input) > tangent_circle_radius_next_cos
		if in_tan2_rad:
			return false

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

func get_on_path_sequence(next, input: Vector3) -> Variant:
	var c1xc2 = control_point.cross(next.control_point)
	var c1c2dir = input.dot(c1xc2)
	if c1c2dir < 0.0:
		var c1xt1 = control_point.cross(tangent_circle_center_next_1)
		var t1xc2 = tangent_circle_center_next_1.cross(next.control_point)
		if input.dot(c1xt1) > 0 and input.dot(t1xc2) > 0:
			var tan1_to_input = IKRay3D.new(tangent_circle_center_next_1, input)
			var result: Vector3
			tan1_to_input.intersects_plane(Vector3(0, 0, 0), control_point, next.control_point, result)
			return result.normalized()
		else:
			return null
	else:
		var t2xc1 = tangent_circle_center_next_2.cross(control_point)
		var c2xt2 = next.control_point.cross(tangent_circle_center_next_2)
		if input.dot(t2xc1) > 0 and input.dot(c2xt2) > 0:
			var tan2_to_input = IKRay3D.new(tangent_circle_center_next_2, input)
			var result: Vector3
			tan2_to_input.intersects_plane(Vector3(0, 0, 0), control_point, next.control_point, result)
			return result.normalized()
		else:
			return null

## Returns null if inapplicable for rectification. The original point if in
## bounds, or the point rectified to the closest boundary on the path
## sequence between two cones if the point is out of bounds and applicable for
## rectification.
func get_on_great_tangent_triangle(next: IKLimitCone, input: Vector3) -> Variant:
	var c1xc2 = control_point.cross(next.control_point)
	var c1c2dir = input.dot(c1xc2)
	
	if c1c2dir < 0.0:
		var c1xt1 = control_point.cross(tangent_circle_center_next_1)
		var t1xc2 = tangent_circle_center_next_1.cross(next.control_point)
		
		if input.dot(c1xt1) > 0 and input.dot(t1xc2) > 0:
			var to_next_cos = input.dot(tangent_circle_center_next_1)
			
			if to_next_cos > tangent_circle_radius_next_cos:
				var plane_normal = tangent_circle_center_next_1.cross(input)
				var rotate_about_by = Quaternion(plane_normal, tangent_circle_radius_next)
				return rotate_about_by.xform(tangent_circle_center_next_1)
			else:
				return input
		else:
			return null
	else:
		var t2xc1 = tangent_circle_center_next_2.cross(control_point)
		var c2xt2 = next.control_point.cross(tangent_circle_center_next_2)
		
		if input.dot(t2xc1) > 0 and input.dot(c2xt2) > 0:
			if input.dot(tangent_circle_center_next_2) > tangent_circle_radius_next_cos:
				var plane_normal = tangent_circle_center_next_2.cross(input)
				var rotate_about_by = Quaternion(plane_normal, tangent_circle_radius_next)
				return rotate_about_by.xform(tangent_circle_center_next_2)
			else:
				return input
		else:
			return null

func closest_cone(next: IKLimitCone, input: Vector3) -> Vector3:
	if input.dot(control_point) > input.dot(next.control_point):
		return control_point
	else:
		return next.control_point

# Returns null if no rectification is required.
func closest_point_on_closest_cone(next: IKLimitCone, input: Vector3, in_bounds: Array) -> Variant:
	var closest_to_first = closest_to_cone(input, in_bounds)
	
	if in_bounds[0]:
		return closest_to_first
	
	var closest_to_second = next.closest_to_cone(input, in_bounds)
	
	if in_bounds[0]:
		return closest_to_second
	
	var cos_to_first = input.dot(closest_to_first)
	var cos_to_second = input.dot(closest_to_second)

	if cos_to_first > cos_to_second:
		return closest_to_first
	else:
		return closest_to_second

# Returns null if no rectification is required.
func closest_to_cone(input: Vector3, in_bounds: Array) -> Variant:
	if input.dot(get_control_point()) > get_radius_cosine():
		in_bounds[0] = true
		return null
	else:
		var axis = get_control_point().cross(input)
		var rot_to = Quaternion(axis, get_radius())
		var result = rot_to.xform(get_control_point())
		in_bounds[0] = false
		return result

func update_tangent_handles(next: IKLimitCone) -> void:
	control_point.normalized()
	update_tangent_and_cushion_handles(next, BOUNDARY)
	update_tangent_and_cushion_handles(next, CUSHION)


func update_tangent_and_cushion_handles(next, mode):
	if next != null:
		var radA = _get_radius(mode)
		var radB = next._get_radius(mode)

		var A = get_control_point().duplicated()
		var B = next.get_control_point().duplicated()

		var arc_normal = A.cross(B)

		# There are an infinite number of circles co-tangent with A and B, every other
		# one of which has a unique radius.
		# 
		# However, we want the radius of our tangent circles to obey the following
		# properties:
		# 1) When the radius of A + B == 0 radians, our tangent circle's radius should
		# = pi/2 radians.
		# In other words, the tangent circle should span a hemisphere.
		# 2) When the radius of A + B == pi radians, our tangent circle's radius should
		# = 0 radians.
		# In other words, when A + B combined are capable of spanning the entire
		# sphere,
		# our tangentCircle should be nothing.
		#
		# Another way to think of this is -- whatever the maximum distance can be
		# between the
		# borders of A and B (presuming their centers are free to move about the circle
		# but their radii remain constant), we want our tangentCircle's diameter to be
		# precisely that distance.

		var t_radius = (PI - (radA + radB)) / 2.0

		# Calculate direction of cones
		var boundary_plus_tangent_radius_A = radA + t_radius
		var boundary_plus_tangent_radius_B = radB + t_radius

		var scaled_axis_A = A * cos(boundary_plus_tangent_radius_A)
		var plane_dir1_A = Quaternion(arc_normal, boundary_plus_tangent_radius_A) * A
		var plane_dir2_A = Quaternion(A, PI / 2.0) * plane_dir1_A

		var scaled_axis_B = B * cos(boundary_plus_tangent_radius_B)
		var plane_dir1_B = Quaternion(arc_normal, boundary_plus_tangent_radius_B) * B
		var plane_dir2_B = Quaternion(B, PI / 2.0) * plane_dir1_B

		var r1B = IKRay3D.new(plane_dir1_B, scaled_axis_B)
		var r2B = IKRay3D.new(plane_dir1_B, plane_dir2_B)

		r1B.elongate(99)
		r2B.elongate(99)

		var intersection1 = r1B.intersects_plane(scaled_axis_A, plane_dir1_A, plane_dir2_A)
		var intersection2 = r2B.intersects_plane(scaled_axis_A, plane_dir1_A, plane_dir2_A)

		var intersection_ray = IKRay3D.new(intersection1, intersection2)
		intersection_ray.elongate(99)

		var sphere_intersect1 = Vector3()
		var sphere_intersect2 = Vector3()
		var sphere_center = Vector3()
		intersection_ray.intersects_sphere(sphere_center, 1.0, sphere_intersect1, sphere_intersect2)

		set_tangent_circle_center_next1(sphere_intersect1, mode)
		set_tangent_circle_center_next2(sphere_intersect2, mode)
		set_tangent_circle_radius_next(t_radius, mode)

	if tangent_circle_center_next_1 == null:
		tangent_circle_center_next_1 = get_vector3_orthogonal(control_point).normalized()
		cushion_tangent_circle_center_next_1 = get_vector3_orthogonal(control_point).normalized()

	if tangent_circle_center_next_2 == null:
		tangent_circle_center_next_2 = (-tangent_circle_center_next_1).normalized()
		cushion_tangent_circle_center_next2 = (-cushion_tangent_circle_center_next_1).normalized()

	if next != null:
		compute_triangles(next)

# Set tangent circle center next1
func set_tangent_circle_center_next1(point, mode):
	if mode == CUSHION:
		cushion_tangent_circle_center_next_1 = point
	else:
		tangent_circle_center_next_1 = point


# Set tangent circle center next2
func set_tangent_circle_center_next2(point, mode):
	if mode == CUSHION:
		cushion_tangent_circle_center_next2 = point
	else:
		tangent_circle_center_next_2 = point


# Set tangent circle radius next
func set_tangent_circle_radius_next(rad, mode):
	if mode == CUSHION:
		cushion_tangent_circle_radius_next = rad
		cushion_tangent_circle_radius_next_cos = cos(cushion_tangent_circle_radius_next)
	else:
		tangent_circle_radius_next = rad
		tangent_circle_radius_next_cos = cos(tangent_circle_radius_next)


## for internal and rendering use only. Avoid modifying any values in the
## resulting object,
## which is returned by reference.
func get_tangent_circle_center_next1(mode):
	if mode == CUSHION:
		return cushion_tangent_circle_center_next_1
	return tangent_circle_center_next_1


# Get tangent circle radius next
func get_tangent_circle_radius_next(mode):
	if mode == CUSHION:
		return cushion_tangent_circle_radius_next
	return tangent_circle_radius_next


# Get tangent circle radius next cosine
func get_tangent_circle_radius_next_cos(mode):
	if mode == CUSHION:
		return cushion_tangent_circle_radius_next_cos
	return tangent_circle_radius_next_cos


# For internal and rendering use only. Avoid modifying any values in the
# resulting object, which is returned by reference.
#
# @param mode: int - The mode to determine which tangent circle center to return
# @return Vector2 - The tangent circle center based on the given mode
func get_tangent_circle_center_next_2(mode: int) -> Vector3:
	if mode == CUSHION:
		return cushion_tangent_circle_center_next2
	return tangent_circle_center_next_2

# Get radius
func _get_radius(mode):
	if mode == CUSHION:
		return cushion_radius
	return radius


# Get radius cosine
func _get_radius_cosine(mode):
	if mode == CUSHION:
		return cushion_cosine
	else:
		return radius_cosine


# Compute triangles
func compute_triangles(next):
	first_triangle_next[1] = tangent_circle_center_next_1.normalized()
	first_triangle_next[0] = get_control_point().normalized()
	first_triangle_next[2] = next.get_control_point().normalized()

	second_triangle_next[1] = tangent_circle_center_next_2.normalized()
	second_triangle_next[0] = get_control_point().normalized()
	second_triangle_next[2] = next.get_control_point().normalized()


# Get control point
func get_control_point():
	return control_point


# Get radius
func get_radius():
	return radius


# Get radius cosine
func get_radius_cosine():
	return radius_cosine


# Set radius
func set_radius(radius):
	self.radius = radius
	self.radius_cosine = cos(radius)
	parent_kusudama.constraint_update_notification()


# Get cushion radius
func get_cushion_radius():
	return cushion_radius / radius


# Get cushion cosine
func get_cushion_cosine():
	return cushion_cosine

# Converts the given cushion value to a range between 0 and 1, where:
# - 0 means the cushion will begin at the center of the cone
# - 1 is equivalent to no cushion
#
# @param cushion: float - The cushion value to be converted
func set_cushion_ratio(cushion):
	var adjusted_cushion = min(1.0, max(0.001, cushion))
	cushion_radius = radius * adjusted_cushion
	cushion_cosine = cos(cushion_radius)
	parent_kusudama.constraint_update_notification()


# Get parent kusudama
func get_parent_kusudama():
	return parent_kusudama
