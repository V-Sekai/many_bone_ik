@tool
extends Reference

class_name IKBasis

enum Axis {
    NONE = -1,
    X = 0,
    Y = 1,
    Z = 2
}

enum Chirality {
    LEFT = -1,
    RIGHT = 1
}

var chirality: int = Chirality.RIGHT

var rotation: Quaternion
var inverse_rotation: Quaternion

var translate: Vector3

var x_base: Vector3
var y_base: Vector3
var z_base: Vector3

var x_ray: IKRay3D
var y_ray: IKRay3D
var z_ray: IKRay3D

func _init(origin: Vector3) -> void:
    translate = origin.duplicate()
    x_base = origin.duplicate()
    y_base = origin.duplicate()
    z_base = origin.duplicate()
    x_base.set(1, 0, 0)
    y_base.set(0, 1, 0)
    z_base.set(0, 0, 1)
    var zero: Vector3 = origin.duplicate()
    zero.set(0, 0, 0)
    x_ray = IKRay3D.new(zero.duplicate(), x_base.duplicate())
    y_ray = IKRay3D.new(zero.duplicate(), y_base.duplicate())
    z_ray = IKRay3D.new(zero.duplicate(), z_base.duplicate())
    refresh_precomputed()

func adopt_values(input: IKBasis) -> void:
    translate.set(input.translate)
    rotation.set(input.rotation)
    x_base = translate.duplicate()
    y_base = translate.duplicate()
    z_base = translate.duplicate()
    x_base.set(1, 0, 0)
    y_base.set(0, 1, 0)
    z_base.set(0, 0, 1)
    x_ray = input.x_ray.duplicate()
    y_ray = input.y_ray.duplicate()
    z_ray = input.z_ray.duplicate()
    refresh_precomputed()

func set_identity() -> void:
    translate.set(0, 0, 0)
    x_base.set(1, 0, 0)
    y_base.set(0, 1, 0)
    z_base.set(0, 0, 1)
    x_ray.p1.set(translate)
    x_ray.p2.set(x_base)
    y_ray.p1.set(translate)
    y_ray.p2.set(y_base)
    z_ray.p1.set(translate)
    z_ray.p2.set(z_base)
    rotation = Quaternion()
    refresh_precomputed()

func create_prioritized_rotation(x_heading: Vector3, y_heading: Vector3, z_heading: Vector3) -> Quaternion:
	var temp_v: Vector3 = z_heading.duplicate()
	temp_v.set(0, 0, 0)
	var to_yz: Quaternion = Quaternion(y_base, z_base, y_heading, z_heading)
	to_yz.apply_to(y_base, temp_v)
	var to_y: Quaternion = Quaternion(temp_v, y_heading)

	return to_y.apply_to(to_yz)

func get_local_of_rotation(in_rot: Quaternion) -> Quaternion:
	var result_new: Quaternion = inverse_rotation.apply_to(in_rot).apply_to(rotation)
	return result_new

func set_to_local_of(global_input: IKBasis, local_output: IKBasis) -> void:
	local_output.translate = get_local_of(global_input.translate)
	inverse_rotation.apply_to(global_input.rotation, local_output.rotation)

	local_output.refresh_precomputed()

func refresh_precomputed() -> void:
	rotation.set_to_reversion(inverse_rotation)
	update_rays()

func get_local_of(v: Vector3) -> Vector3:
	var result: Vector3 = v.duplicate()
	set_to_local_of(v, result)
	return result

func set_to_local_of(input: Vector3, output: Vector3) -> void:
	output.set(input)
	output -= translate
	inverse_rotation.apply_to(output, output)

func rotate_to(new_rotation: Quaternion) -> void:
	rotation.set(new_rotation)
	refresh_precomputed()

func rotate_by(add_rotation: Quaternion) -> void:
	add_rotation.apply_to(rotation, rotation)
	refresh_precomputed()

func set_to_shear_x_base(vec: Vector3) -> void:
	vec.set(x_base)

func set_to_shear_y_base(vec: Vector3) -> void:
	vec.set(y_base)

func set_to_shear_z_base(vec: Vector3) -> void:
	vec.set(z_base)

func set_to_global_of(local_input: IKBasis, global_output: IKBasis) -> void:
	rotation.apply_to(local_input.rotation, global_output.rotation)
	set_to_global_of(local_input.translate, global_output.translate)
	global_output.refresh_precomputed()

func set_to_global_of(input: Vector3, output: Vector3) -> void:
	rotation.apply_to(input, output)
	output += translate

func translate_by(trans_by: Vector3) -> void:
	translate += trans_by
	update_rays()

func translate_to(new_origin: Vector3) -> void:
	translate = new_origin
	update_rays()

func get_x_ray() -> IKRay3D:
	return x_ray

func get_y_ray() -> IKRay3D:
	return y_ray

func get_z_ray() -> IKRay3D:
	return z_ray

func get_x_heading() -> Vector3:
	return x_ray.heading()

func get_y_heading() -> Vector3:
	return y_ray.heading()

func get_z_heading() -> Vector3:
	return z_ray.heading()

func get_origin() -> Vector3:
	return translate

func is_axis_flipped(axis: int) -> bool:
	return false

func get_inverse_rotation() -> Quaternion:
	return inverse_rotation

func update_rays() -> void:
	x_ray.set_p1(translate)
	x_ray.p2.set(x_base)
	y_ray.set_p1(translate)
	y_ray.p2.set(y_base)
	z_ray.set_p1(translate)
	z_ray.p2.set(z_base)

	rotation.apply_to(x_ray.p2, x_ray.p2)
	rotation.apply_to(y_ray.p2, y_ray.p2)
	rotation.apply_to(z_ray.p2, z_ray.p2)

	x_ray.p2 += translate
	y_ray.p2 += translate
	z_ray.p2 += translate

func print_basis() -> String:
	var xh: Vector3 = x_ray.heading()
	var yh: Vector3 = y_ray.heading()
	var zh: Vector3 = z_ray.heading()

	var x_mag: float = xh.length()
	var y_mag: float = yh.length()
	var z_mag: float = zh.length()
	var chirality: String = chirality == LEFT ? "LEFT" : "RIGHT"
	var result: String = "-----------\n" \
		+ chirality + " handed \n" \
		+ "origin: " + translate + "\n" \
		+ "rot Axis: " + rotation.get_axis() + ", " \
		+ "Angle: " + rad2deg(rotation.get_angle()) + "\n" \
		+ "xHead: " + xh + ", mag: " + x_mag + "\n" \
		+ "yHead: " + yh + ", mag: " + y_mag + "\n" \
		+ "zHead: " + zh + ", mag: " + z_mag + "\n"

	return result