# Copyright (c) 2018-present. This file is part of V-Sekai https://v-sekai.org/.
# K. S. Ernest (Fire) Lee & Contributors
# qcp.gd
# SPDX-License-Identifier: MIT

extends Node

class_name QCP

## Precision for eigenvector calculation
var eigenvector_precision = 1e-6

## Arrays to hold target and moved vectors
var target : Array
var moved : Array

## Weight array and sum
var weight : Array
var w_sum = 0

## Centers of target and moved vectors
var target_center : Vector3
var moved_center : Vector3

## Variables for various sums used in calculations
var sum_xy = 0
var sum_xz = 0
var sum_yx = 0
var sum_yz = 0
var sum_zx = 0
var sum_zy = 0
var sum_xx_plus_yy = 0
var sum_zz = 0
var max_eigenvalue = 0
var sum_yz_minus_zy = 0
var sum_xz_minus_zx = 0
var sum_xy_minus_yx = 0
var sum_xx_minus_yy = 0
var sum_xy_plus_yx = 0
var sum_xz_plus_zx = 0
var sum_yy = 0
var sum_xx = 0
var sum_yz_plus_zy = 0

## Flags to check if transformation and inner product have been calculated
var inner_product_calculated = false

## Constructor method to set eigenvector precision
func _init(p_evec_prec: float):
	eigenvector_precision = p_evec_prec

## Method to calculate inner product
func inner_product(coords1 : Array, coords2 : Array):
	var weighted_coord1 : Vector3
	var weighted_coord2 : Vector3
	var sum_of_squares1 = 0.0
	var sum_of_squares2 = 0.0

	sum_xx = 0.0
	sum_xy = 0.0
	sum_xz = 0.0
	sum_yx = 0.0
	sum_yy = 0.0
	sum_yz = 0.0
	sum_zx = 0.0
	sum_zy = 0.0
	sum_zz = 0.0

	var weight_is_empty = weight.is_empty()
	var size = coords1.size()

	for i in range(size):
		if not weight_is_empty:
			weighted_coord1 = weight[i] * coords1[i]
			sum_of_squares1 += weighted_coord1.dot(coords1[i])
		else:
			weighted_coord1 = coords1[i]
			sum_of_squares1 += weighted_coord1.dot(weighted_coord1)

		weighted_coord2 = coords2[i]

		if weight_is_empty:
			sum_of_squares2 += weighted_coord2.dot(weighted_coord2);
		else:
			sum_of_squares2 += weight[i] * weighted_coord2.dot(weighted_coord2);

		sum_xx += (weighted_coord1.x * weighted_coord2.x)
		sum_xy += (weighted_coord1.x * weighted_coord2.y)
		sum_xz += (weighted_coord1.x * weighted_coord2.z)

		sum_yx += (weighted_coord1.y * weighted_coord2.x)
		sum_yy += (weighted_coord1.y * weighted_coord2.y)
		sum_yz += (weighted_coord1.y * weighted_coord2.z)

		sum_zx += (weighted_coord1.z * weighted_coord2.x)
		sum_zy += (weighted_coord1.z * weighted_coord2.y)
		sum_zz += (weighted_coord1.z * weighted_coord2.z)

	var initial_eigenvalue = (sum_of_squares1 + sum_of_squares2) * 0.5

	sum_xz_plus_zx = sum_xz + sum_zx
	sum_yz_plus_zy = sum_yz + sum_zy
	sum_xy_plus_yx = sum_xy + sum_yx
	sum_yz_minus_zy = sum_yz - sum_zy
	sum_xz_minus_zx = sum_xz - sum_zx
	sum_xy_minus_yx = sum_xy - sum_yx
	sum_xx_plus_yy = sum_xx + sum_yy
	sum_xx_minus_yy = sum_xx - sum_yy
	max_eigenvalue = initial_eigenvalue

	inner_product_calculated = true

## Method to calculate rotation
func calculate_rotation() -> Quaternion:
	var result = Quaternion()
	if moved.size() == 1:
		var u = moved[0]
		var v = target[0]
		var norm_product = u.length() * v.length()

		if norm_product == 0.0:
			return Quaternion()

		var dot = u.dot(v)

		if dot < ((2.0e-15 - 1.0) * norm_product):
			var w = u.normalized()
			result = Quaternion(w.x, w.y, w.z, 0.0).normalized()
		else:
			var q0 = sqrt(0.5 * (1.0 + dot / norm_product))
			var coeff = 1.0 / (2.0 * q0 * norm_product)
			var q = v.cross(u).normalized()
			result = Quaternion(coeff * q.x, coeff * q.y, coeff * q.z, q0).normalized()
	else:
		var a13 = -sum_xz_minus_zx
		var a14 = sum_xy_minus_yx
		var a21 = sum_yz_minus_zy
		var a22 = sum_xx_minus_yy - sum_zz - max_eigenvalue
		var a23 = sum_xy_plus_yx
		var a24 = sum_xz_plus_zx
		var a31 = a13
		var a32 = a23
		var a33 = sum_yy - sum_xx - sum_zz - max_eigenvalue
		var a34 = sum_yz_plus_zy
		var a41 = a14
		var a42 = a24
		var a43 = a34
		var a44 = sum_zz - sum_xx_plus_yy - max_eigenvalue

		var a3344_4334 = a33 * a44 - a43 * a34
		var a3244_4234 = a32 * a44 - a42 * a34
		var a3243_4233 = a32 * a43 - a42 * a33
		var a3143_4133 = a31 * a43 - a41 * a33
		var a3144_4134 = a31 * a44 - a41 * a34
		var a3142_4132 = a31 * a42 - a41 * a32

		var quaternion_w = a22 * a3344_4334 - a23 * a3244_4234 + a24 * a3243_4233
		var quaternion_x = -a21 * a3344_4334 + a23 * a3144_4134 - a24 * a3143_4133
		var quaternion_y = a21 * a3244_4234 - a22 * a3144_4134 + a24 * a3142_4132
		var quaternion_z = -a21 * a3243_4233 + a22 * a3143_4133 - a23 * a3142_4132
		var qsqr = quaternion_w * quaternion_w + quaternion_x * quaternion_x + quaternion_y * quaternion_y + quaternion_z * quaternion_z

		if qsqr < eigenvector_precision:
			result = Quaternion()
		else:
			quaternion_x *= -1
			quaternion_y *= -1
			quaternion_z *= -1
			var min = quaternion_w
			if quaternion_x < min:
				min = quaternion_x;
			if quaternion_y < min:
				min = quaternion_y
			if quaternion_z < min:
				min = quaternion_z;
			quaternion_w /= min
			quaternion_x /= min
			quaternion_y /= min
			quaternion_z /= min
			result = Quaternion(quaternion_x, quaternion_y, quaternion_z, quaternion_w).normalized()

	return result



## Method to translate a vector array by a given vector
func translate(r_translate : Vector3, r_x : Array):
	for i in range(r_x.size()):
		r_x[i] += r_translate


## Method to get translation
func get_translation() -> Vector3:
	return target_center - moved_center


## Method to move to weighted center
func move_to_weighted_center(r_to_center : Array, r_weight : Array) -> Vector3:
	var center = Vector3()
	var total_weight = 0.0
	var weight_is_empty = r_weight.is_empty()

	for i in range(r_to_center.size()):
		if not weight_is_empty:
			total_weight += r_weight[i]
			center += r_to_center[i] * r_weight[i]
		else:
			center += r_to_center[i]
			total_weight += 1

	if total_weight > 0:
		center /= total_weight

	return center


func get_rotation() -> Quaternion:
	var result: Quaternion
	if not inner_product_calculated:
		inner_product(target, moved)
	result = calculate_rotation()
	return result


## Method to superpose with weights
func weighted_superpose(p_moved : Array, p_target : Array, p_weight : Array, p_translate : bool) -> Quaternion:
	inner_product_calculated = false

	moved = p_moved
	target = p_target
	weight = p_weight
	if p_translate:
		moved_center = move_to_weighted_center(moved, weight)
		w_sum = 0 # set wsum to 0 so we don't double up.
		target_center = move_to_weighted_center(target, weight)
		translate(moved_center * -1, moved)
		translate(target_center * -1, target)
	else:
		if not weight.is_empty():
			for w in weight:
				w_sum += w
	return get_rotation()
