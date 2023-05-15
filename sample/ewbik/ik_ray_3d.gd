@tool
extends Object

class_name IKRay3D

const X = 0
const Y = 1
const Z = 2
var p1: Vector3
var p2: Vector3
var working_vector: Vector3

func _init(origin: Vector3, end_point: Vector3) -> void:
	self.working_vector = origin
	self.p1 = origin
	self.p2 = end_point

func heading() -> Vector3:
	self.working_vector = p2
	return working_vector - p1

func set_heading(new_head: Vector3) -> void:
	self.p2 = self.p1 + new_head

func get_heading(set_to: Vector3) -> void:
	set_to = p2 - p1

func origin() -> Vector3:
	return p1

func mag() -> float:
	working_vector = p2
	return (working_vector - p1).length()

func set_mag(new_mag: float) -> void:
	working_vector = p2
	var dir = working_vector - p1
	dir = dir.normalized() * new_mag
	set_heading(dir)

func div(divisor: float) -> void:
	p2 -= p1
	p2 /= divisor
	p2 += p1

func mult(scalar: float) -> void:
	p2 -= p1
	p2 *= scalar
	p2 += p1

func get_multiplied_by(scalar: float) -> Vector3:
	var result = heading()
	result *= scalar
	result += p1
	return result

func get_divided_by(divisor: float) -> Vector3:
	var result = heading()
	result /= divisor
	result += p1
	return result

func get_scaled_to(scale: float) -> Vector3:
	var result = heading()
	result.normalize()
	result *= scale
	result += p1
	return result

func elongate(amt: float) -> void:
	var mid_point = (p1 + p2) * 0.5
	var p1_heading = p1 - mid_point
	var p2_heading = p2 - mid_point
	var p1_add = p1_heading.normalized() * amt
	var p2_add = p2_heading.normalized() * amt

	p1 = p1_heading + p1_add + mid_point
	p2 = p2_heading + p2_add + mid_point

func copy() -> IKRay3D:
	return IKRay3D.new(p1, p2)

func reverse() -> void:
	var temp = p1
	p1 = p2
	p2 = temp

func get_reversed() -> IKRay3D:
	return IKRay3D.new(p2, p1)

func get_ray_scaled_to(scalar: float) -> IKRay3D:
	return IKRay3D.new(p1, get_scaled_to(scalar))

func point_with(r: IKRay3D) -> void:
	if heading().dot(r.heading()) < 0:
		reverse()

func point_with_heading(heading: Vector3) -> void:
	if self.heading().dot(heading) < 0:
		reverse()

func get_ray_scaled_by(scalar: float) -> IKRay3D:
	return IKRay3D.new(p1, get_multiplied_by(scalar))

func set_to_inverted_tip(vec: Vector3) -> Vector3:
	vec.x = (p1.x - p2.x) + p1.x
	vec.y = (p1.y - p2.y) + p1.y
	vec.z = (p1.z - p2.z) + p1.z
	return vec

func contract_to(percent: float) -> void:
	var half_percent = 1 - ((1 - percent) / 2.0)

	p1 = p1.lerp(p2, half_percent)
	p2 = p2.lerp(p1, half_percent)

func translate_to(new_location: Vector3) -> void:
	working_vector = p2 - p1
	working_vector += new_location
	p2 = working_vector
	p1 = new_location

func translate_tip_to(new_location: Vector3) -> void:
	working_vector = new_location
	var trans_by = working_vector - p2
	translate_by(trans_by)

func translate_by(to_add: Vector3) -> void:
	p1 += to_add
	p2 += to_add

func normalize() -> void:
	set_mag(1)

static func tri_area_2d(x1: float, y1: float, x2: float, y2: float, x3: float, y3: float) -> float:
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2)

var I: Vector3
var u: Vector3
var v: Vector3
var n: Vector3
var dir: Vector3
var w0: Vector3
var in_use: bool = false

func barycentric(a: Vector3, b: Vector3, c: Vector3, p: Vector3, uvw: Array) -> void:
	bc = b
	ca = a
	at = a
	bt = b
	ct = c
	pt = p

	m = ((bc - ct).cross(ca - at))

	var nu: float
	var nv: float
	var ood: float

	var x = abs(m.x)
	var y = abs(m.y)
	var z = abs(m.z)

	if x >= y and x >= z:
		nu = tri_area_2d(pt.y, pt.z, bt.y, bt.z, ct.y, ct.z)
		nv = tri_area_2d(pt.y, pt.z, ct.y, ct.z, at.y, at.z)
		ood = 1.0 / m.x
	elif y >= x and y >= z:
		nu = tri_area_2d(pt.x, pt.z, bt.x, bt.z, ct.x, ct.z)
		nv = tri_area_2d(pt.x, pt.z, ct.x, ct.z, at.x, at.z)
		ood = 1.0 / -m.y
	else:
		nu = tri_area_2d(pt.x, pt.y, bt.x, bt.y, ct.x, ct.y)
		nv = tri_area_2d(pt.x, pt.y, ct.x, ct.y, at.x, at.y)
		ood = 1.0 / m.z

	uvw[0] = nu * ood
	uvw[1] = nv * ood
	uvw[2] = 1.0 - uvw[0] - uvw[1]

func plane_intersect_test(ta: Vector3, tb: Vector3, tc: Vector3, uvw: Array) -> Vector3:
	u = tb
	v = tc
	n = Vector3()
	dir = heading()
	w0 = Vector3(0, 0, 0)

	var r: float
	var a: float
	var b: float
	u -= ta
	v -= ta

	n = u.cross(v)

	w0 -= ta
	a = -(n.dot(w0))
	b = n.dot(dir)
	r = a / b
	I = Vector3(0, 0, 0)
	I = dir
	I *= r
	barycentric(ta, tb, tc, I, uvw)

	return I

func intersects_sphere(sphere_center: Vector3, radius: float, S1: Vector3, S2: Vector3) -> int:
	var tp1 = p1 - sphere_center
	var tp2 = p2 - sphere_center
	var result = intersects_sphere_vectors(tp1, tp2, radius, S1, S2)
	S1 += sphere_center
	S2 += sphere_center
	return result

func intersects_sphere_vectors(rp1: Vector3, rp2: Vector3, radius: float, S1: Vector3, S2: Vector3) -> int:
	var direction: Vector3 = rp2 - rp1
	var e = direction
	e.normalize()
	var h = p1
	h.set(0, 0, 0)
	h -= rp1
	var lf = e.dot(h)
	var radpow = radius * radius
	var hdh = h.length_squared()
	var lfpow = lf * lf
	var s = radpow - hdh + lfpow

	if s < 0.0:
		return 0

	s = sqrt(s)
	var result: int

	if lf < s and lf + s >= 0:
		s = -s
		result = 1
	else:
		result = 2

	S1 = e * (lf - s)
	S1 += rp1
	S2 = e * (lf + s)
	S2 += rp1

	return result

var m: Vector3
var at: Vector3
var bt: Vector3
var ct: Vector3
var pt: Vector3
var bc: Vector3
var ca: Vector3
var ac: Vector3

func _print() -> String:
	var result = "IKRay3D (%s ->  %s)\n(%s ->  %s)\n(%s ->  %s)\n" % [
		str(p1.x), str(p2.x),
		str(p1.y), str(p2.y),
		str(p1.z), str(p2.z)
	]
	return result

func set_p1(input: Vector3) -> void:
	p1 = input

func set_p2(input: Vector3) -> void:
	p2 = input

func lerp(a: float, b: float, t: float) -> float:
	return (1 - t) * a + t * b

func get_p2() -> Vector3:
	return p2

func set_from_ik_ray(r: IKRay3D) -> void:
	p1 = r.p1
	p2 = r.p2

func get_p1() -> Vector3:
	return p1
