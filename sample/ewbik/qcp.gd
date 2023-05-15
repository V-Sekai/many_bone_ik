## Implementation of the Quaternionf-Based Characteristic Polynomial algorithm
## for RMSD and Superposition calculations.
##
## Citations:
##
## Liu P, Agrafiotis DK, & Theobald DL (2011) Reply to comment on: "Fast
## determination of the optimal Quaternionation matrix for macromolecular
## superpositions." Journal of Computational Chemistry 32(1):185-186.
## [http://dx.doi.org/10.1002/jcc.21606]
##
## Liu P, Agrafiotis DK, & Theobald DL (2010) "Fast determination of the optimal
## Quaternionation matrix for macromolecular superpositions." Journal of Computational
## Chemistry 31(7):1561-1563. [http://dx.doi.org/10.1002/jcc.21439]
##
## Douglas L Theobald (2005) "Rapid calculation of RMSDs using a
## quaternion-based characteristic polynomial." Acta Crystallogr A
## 61(4):478-480. [http://dx.doi.org/10.1107/S0108767305015266 ]
##
## This is an adoption of the original C code QCPQuaternion 1.4 (2012, October 10) to
## Java. The original C source code is available from
## http://theobald.brandeis.edu/qcp/ and was developed by
##
## Douglas L. Theobald Department of Biochemistry MS 009 Brandeis University 415
## South St Waltham, MA 02453 USA
##
## dtheobald@brandeis.edu
##
## Pu Liu Johnson & Johnson Pharmaceutical Research and Development, L.L.C. 665
## Stockton Drive Exton, PA 19341 USA
##
## pliu24@its.jnj.com
##
## @author Douglas L. Theobald (original C code)
## @author Pu Liu (original C code)
## @author Peter Rose (adopted to Java)
## @author Aleix Lafita (adopted to Java)
## @author Eron Gjoni (adopted to EWB IK)
## @author K. S. Ernest (iFire) Lee (adopted to Godot Engine 4.0)

@tool
extends RefCounted

class_name QCP

var evec_prec: float = 1e-6
var eval_prec: float = 1e-11

var target: PackedVector3Array
var moved: PackedVector3Array

var weight: PackedFloat64Array
var w_sum: float = 0

var target_center: Vector3
var moved_center: Vector3

var e0: float = 0
var rmsd: float = 0
var Sxy: float = 0
var Sxz: float = 0
var Syx: float = 0
var Syz: float = 0
var Szx: float = 0
var Szy: float = 0
var SxxpSyy: float = 0
var Szz: float = 0
var mxEigenV: float = 0
var SyzmSzy: float = 0
var SxzmSzx: float = 0
var SxymSyx: float = 0
var SxxmSyy: float = 0
var SxypSyx: float = 0
var SxzpSzx: float = 0
var Syy: float = 0
var Sxx: float = 0
var SyzpSzy: float = 0
var rmsd_calculated: bool = false
var transformation_calculated: bool = false
var inner_product_calculated: bool = false

func _init(p_evec_prec: float, p_eval_prec: float):
	self.evec_prec = p_evec_prec
	self.eval_prec = p_eval_prec

func calculate_rmsd_array(x: PackedVector3Array, y: PackedVector3Array) -> void:
	if x.size() == 1:
		rmsd = x[0].distance_to(y[0])
		rmsd_calculated = true
	else:
		if not inner_product_calculated:
			inner_product(y, x)
		calculate_rmsd(w_sum)

func inner_product(coords1: PackedVector3Array, coords2: PackedVector3Array) -> void:
	var x1: float = 0
	var x2: float = 0
	var y1: float = 0
	var y2: float = 0
	var z1: float = 0
	var z2: float = 0
	var g1: float = 0
	var g2: float = 0

	Sxx = 0
	Sxy = 0
	Sxz = 0
	Syx = 0
	Syy = 0
	Syz = 0
	Szx = 0
	Szy = 0
	Szz = 0

	if not weight.is_empty():
		for i in range(coords1.size()):
			x1 = weight[i] * coords1[i].x
			y1 = weight[i] * coords1[i].y
			z1 = weight[i] * coords1[i].z

			g1 += x1 * coords1[i].x + y1 * coords1[i].y + z1 * coords1[i].z

			x2 = coords2[i].x
			y2 = coords2[i].y
			z2 = coords2[i].z

			g2 += weight[i] * (x2 * x2 + y2 * y2 + z2 * z2)

			Sxx += (x1 * x2)
			Sxy += (x1 * y2)
			Sxz += (x1 * z2)

			Syx += (y1 * x2)
			Syy += (y1 * y2)
			Syz += (y1 * z2)

			Szx += (z1 * x2)
			Szy += (z1 * y2)
			Szz += (z1 * z2)
	else:
		for i in range(coords1.size()):
			g1 += coords1[i].x * coords1[i].x + coords1[i].y * coords1[i].y + coords1[i].z * coords1[i].z
			g2 += coords2[i].x * coords2[i].x + coords2[i].y * coords2[i].y + coords2[i].z * coords2[i].z

			Sxx += coords1[i].x * coords2[i].x
			Sxy += coords1[i].x * coords2[i].y
			Sxz += coords1[i].x * coords2[i].z

			Syx += coords1[i].y * coords2[i].x
			Syy += coords1[i].y * coords2[i].y
			Syz += coords1[i].y * coords2[i].z

			Szx += coords1[i].z * coords2[i].x
			Szy += coords1[i].z * coords2[i].y
			Szz += coords1[i].z * coords2[i].z

	e0 = (g1 + g2) * 0.5

	SxzpSzx = Sxz + Szx
	SyzpSzy = Syz + Szy
	SxypSyx = Sxy + Syx
	SyzmSzy = Syz - Szy
	SxzmSzx = Sxz - Szx
	SxymSyx = Sxy - Syx
	SxxpSyy = Sxx + Syy
	SxxmSyy = Sxx - Syy
	mxEigenV = e0

	inner_product_calculated = true

func calculate_rmsd(r_length: float) -> void:
	rmsd = sqrt(abs(2.0 * (e0 - mxEigenV) / r_length))

func set_basic(r_target: PackedVector3Array, r_moved: PackedVector3Array) -> void:
	target = r_target
	moved = r_moved
	rmsd_calculated = false
	transformation_calculated = false
	inner_product_calculated = false

func set_advanced(p_moved: PackedVector3Array, p_target: PackedVector3Array, p_weight: PackedFloat64Array, p_translate: bool) -> void:
	rmsd_calculated = false
	transformation_calculated = false
	inner_product_calculated = false

	moved = p_moved
	target = p_target
	weight = p_weight

	if p_translate:
		moved_center = move_to_weighted_center(moved, weight)
		w_sum = 0
		target_center = move_to_weighted_center(target, weight)
		translate(moved_center * -1, moved)
		translate(target_center * -1, target)
	else:
		if not p_weight.is_empty():
			for i in range(p_weight.size()):
				w_sum += p_weight[i]
		else:
			w_sum = p_moved.size()
	
func calculate_rotation() -> Quaternion:
	# QCP doesn't handle single targets, so if we only have one point and one
	# target, we just rotate by the angular distance between them
	if moved.size() == 1:
		var u: Vector3 = moved[0]
		var v: Vector3 = target[0]
		var norm_product: float = u.length() * v.length()
		if norm_product == 0.0:
			return Quaternion()

		var dot: float = u.dot(v)
		if dot < ((2.0e-15 - 1.0) * norm_product):
			var w: Vector3 = u.normalized()
			return Quaternion(-1 * -w.x, -1 * -w.y, -1 * -w.z, 0.0).normalized()

		var q0: float = sqrt(0.5 * (1.0 + dot / norm_product))
		var coeff: float = 1.0 / (2.0 * q0 * norm_product)
		var q: Vector3 = v.cross(u)
		var q1: float = coeff * q.x
		var q2: float = coeff * q.y
		var q3: float = coeff * q.z
		return Quaternion(-1 * q1, -1 * q2, -1 * q3, q0).normalized()
	else:
		var a11: float = SxxpSyy + Szz - mxEigenV
		var a12: float = SyzmSzy
		var a13: float = -SxzmSzx
		var a14: float = SxymSyx
		var a21: float = SyzmSzy
		var a22: float = SxxmSyy - Szz - mxEigenV
		var a23: float = SxypSyx
		var a24: float = SxzpSzx
		var a31: float = a13
		var a32: float = a23
		var a33: float = Syy - Sxx - Szz - mxEigenV
		var a34: float = SyzpSzy
		var a41: float = a14
		var a42: float = a24
		var a43: float = a34
		var a44: float = Szz - SxxpSyy - mxEigenV
		var a3344_4334: float = a33 * a44 - a43 * a34
		var a3244_4234: float = a32 * a44 - a42 * a34
		var a3243_4233: float = a32 * a43 - a42 * a33
		var a3143_4133: float = a31 * a43 - a41 * a33
		var a3144_4134: float = a31 * a44 - a41 * a34
		var a3142_4132: float = a31 * a42 - a41 * a32
		var q1: float = a22 * a3344_4334 - a23 * a3244_4234 + a24 * a3243_4233
		var q2: float = -a21 * a3344_4334 + a23 * a3144_4134 - a24 * a3143_4133
		var q3: float = a21 * a3244_4234 - a22 * a3144_4134 + a24 * a3142_4132
		var q4: float = -a21 * a3243_4233 + a22 * a3143_4133 - a23 * a3142_4132

		var qsqr: float = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4

		if qsqr < evec_prec:
			q1 = a12 * a3344_4334 - a13 * a3244_4234 + a14 * a3243_4233
			q2 = -a11 * a3344_4334 + a13 * a3144_4134 - a14 * a3143_4133
			q3 = a11 * a3244_4234 - a12 * a3144_4134 + a14 * a3142_4132
			q4 = -a11 * a3243_4233 + a12 * a3143_4133 - a13 * a3142_4132
			qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4

			if qsqr < evec_prec:
				var a1324_1423: float = a13 * a24 - a14 * a23
				var a1224_1422: float = a12 * a24 - a14 * a22
				var a1223_1322: float = a12 * a23 - a13 * a22
				var a1124_1421: float = a11 * a24 - a14 * a21
				var a1123_1321: float = a11 * a23 - a13 * a21
				var a1122_1221: float = a11 * a22 - a12 * a21

				q1 = a42 * a1324_1423 - a43 * a1224_1422 + a44 * a1223_1322
				q2 = -a41 * a1324_1423 + a43 * a1124_1421 - a44 * a1123_1321
				q3 = a41 * a1224_1422 - a42 * a1124_1421 + a44 * a1122_1221
				q4 = -a41 * a1223_1322 + a42 * a1123_1321 - a43 * a1122_1221
				qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4

				if qsqr < evec_prec:
					q1 = a32 * a1324_1423 - a33 * a1224_1422 + a34 * a1223_1322
					q2 = -a31 * a1324_1423 + a33 * a1124_1421 - a34 * a1123_1321
					q3 = a31 * a1224_1422 - a32 * a1124_1421 + a34 * a1122_1221
					q4 = -a31 * a1223_1322 + a32 * a1123_1321 - a33 * a1122_1221
					qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4

					if qsqr < evec_prec:
						return Quaternion()

		q2 *= -1
		q3 *= -1
		q4 *= -1
		var min_val: float = min(min(min(q1, q2), q3), q4)
		q1 /= min_val
		q2 /= min_val
		q3 /= min_val
		q4 /= min_val
		return Quaternion(q2, q3, q4, q1).normalized()

static func translate(r_translate: Vector3, r_x: PackedVector3Array) -> void:
	for i in range(r_x.size()):
		r_x[i] += r_translate

func get_rmsd_array(r_fixed: PackedVector3Array, r_moved: PackedVector3Array) -> float:
	set_basic(r_fixed, r_moved)
	return get_rmsd()

func move_to_weighted_center(r_to_center: PackedVector3Array, r_weight: PackedFloat64Array) -> Vector3:
	var center: Vector3
	var weight_is_empty: bool = r_weight.is_empty()
	if not weight_is_empty:
		for i in range(r_to_center.size()):
			w_sum += r_weight[i]
	if not weight_is_empty and w_sum > 0:
		for i in range(r_to_center.size()):
			center += r_to_center[i] * r_weight[i]
		center /= w_sum
	else:
		w_sum = 0
		for i in range(r_to_center.size()):
			center += r_to_center[i]
			w_sum += 1
		center /= w_sum
	return center

func get_rmsd() -> float:
	if not rmsd_calculated:
		calculate_rmsd_array(moved, target)
		rmsd_calculated = true
	return rmsd

func weighted_superpose(p_moved: PackedVector3Array, p_target: PackedVector3Array, p_weight: PackedFloat64Array, translate: bool) -> Quaternion:
	set_advanced(p_moved, p_target, p_weight, translate)
	return get_rotation()

func get_rotation() -> Quaternion:
	var result: Quaternion
	if not transformation_calculated:
		if not inner_product_calculated:
			inner_product(target, moved)
		result = calculate_rotation()
		transformation_calculated = true
	return result

func get_translation() -> Vector3:
	return target_center - moved_center
