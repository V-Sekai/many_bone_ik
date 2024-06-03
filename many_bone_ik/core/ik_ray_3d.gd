extends RefCounted
class_name IKRay3D

var tta: Vector3
var ttb: Vector3
var ttc: Vector3
var I: Vector3
var u: Vector3
var v: Vector3
var n: Vector3
var dir: Vector3
var w0: Vector3
var m: Vector3
var at: Vector3
var bt: Vector3
var ct: Vector3
var pt: Vector3
var bc: Vector3
var ca: Vector3
var ac: Vector3

var point_1: Vector3
var point_2: Vector3
var working_vector: Vector3

func _init(p_point_one: Vector3 = Vector3(), p_point_two: Vector3 = Vector3()):
	point_1 = p_point_one
	point_2 = p_point_two

func get_heading() -> Vector3:
	return point_2 - point_1

func set_heading(p_new_head: Vector3) -> void:
	point_2 = point_1 + p_new_head

func get_intersects_plane(p_vertex_a: Vector3, p_vertex_b: Vector3, p_vertex_c: Vector3) -> Vector3:
	var uvw : Vector3
	var tta = p_vertex_a
	var ttb = p_vertex_b
	var ttc = p_vertex_c
	tta -= point_1
	ttb -= point_1
	ttc -= point_1
	var result = plane_intersect_test(tta, ttb, ttc)
	return result + point_1

func set_point_1(p_point: Vector3) -> void:
	point_1 = p_point

func set_point_2(p_point: Vector3) -> void:
	point_2 = p_point

func get_point_2() -> Vector3:
	return point_2

func get_point_1() -> Vector3:
	return point_1


func intersects_sphere(sphere_center: Vector3, radius: float) -> Array:
	var tp1 = point_1 - sphere_center
	var tp2 = point_2 - sphere_center
	var S1 = Vector3()
	var S2 = Vector3()
	var result = check_intersection(tp1, tp2, radius, S1, S2)
	S1 += sphere_center
	S2 += sphere_center
	return [result, S1, S2]

func check_intersection(rp1: Vector3, rp2: Vector3, radius: float, S1: Vector3, S2: Vector3) -> int:
	var direction = rp2 - rp1
	var e = direction.normalized()
	var h = Vector3(0.0, 0.0, 0.0) - rp1
	var lf = e.dot(h)
	var radpow = radius * radius
	var hdh = h.length_squared()
	var lfpow = lf * lf
	var s = radpow - hdh + lfpow

	if s < 0.0:
		return 0

	s = sqrt(s)

	var result = 0
	if lf < s:
		if lf + s >= 0:
			s = -s
			result = 1
	else:
		result = 2

	S1 = e * (lf - s) + rp1
	S2 = e * (lf + s) + rp1

	return result

func triangle_area_2d(p_x1: float, p_y1: float, p_x2: float, p_y2: float, p_x3: float, p_y3: float) -> float:
	return (p_x1 - p_x2) * (p_y2 - p_y3) - (p_x2 - p_x3) * (p_y1 - p_y2)

func barycentric(p_a: Vector3, p_b: Vector3, p_c: Vector3, p_p: Vector3) -> Vector3:
	bc = p_b
	ca = p_a
	at = p_a
	bt = p_b
	ct = p_c
	pt = p_p

	m = (bc - ct).cross(ca - at).normalized()

	var nu: float
	var nv: float
	var ood: float

	var x = abs(m.x)
	var y = abs(m.y)
	var z = abs(m.z)

	if x >= y and x >= z:
		nu = triangle_area_2d(pt.y, pt.z, bt.y, bt.z, ct.y, ct.z)
		nv = triangle_area_2d(pt.y, pt.z, ct.y, ct.z, at.y, at.z)
		ood = 1.0 / m.x
	elif y >= x and y >= z:
		nu = triangle_area_2d(pt.x, pt.z, bt.x, bt.z, ct.x, ct.z)
		nv = triangle_area_2d(pt.x, pt.z, ct.x, ct.z, at.x, at.z)
		ood = 1.0 / -m.y
	else:
		nu = triangle_area_2d(pt.x, pt.y, bt.x, bt.y, ct.x, ct.y)
		nv = triangle_area_2d(pt.x, pt.y, ct.x, ct.y, at.x, at.y)
		ood = 1.0 / m.z
	var r_uvw: Vector3
	r_uvw[0] = nu * ood
	r_uvw[1] = nv * ood
	r_uvw[2] = 1.0 - r_uvw[0] - r_uvw[1]
	return r_uvw


func plane_intersect_test(p_vertex_a: Vector3, p_vertex_b: Vector3, p_vertex_c: Vector3) -> Array:
	var u = p_vertex_b
	var v = p_vertex_c
	var n = Vector3(0, 0, 0)
	var dir = get_heading()
	var w0 = Vector3(0, 0, 0)
	var r
	var a
	var b

	u -= p_vertex_a
	v -= p_vertex_a

	n = u.cross(v).normalized()

	w0 -= p_vertex_a
	a = -(n.dot(w0))
	b = n.dot(dir)
	r = a / b

	var I = dir
	I *= r

	var uvw = barycentric(p_vertex_a, p_vertex_b, p_vertex_c, I)

	return [I, uvw]


func _to_string() -> String:
	return "(" + str(point_1.x) + " ->  " + str(point_2.x) + ") \n (" + str(point_1.y) + " ->  " + str(point_2.y) + ") \n (" + str(point_1.z) + " ->  " + str(point_2.z) + ") \n "
