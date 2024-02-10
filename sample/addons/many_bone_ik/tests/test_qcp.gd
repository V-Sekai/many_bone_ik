# Copyright (c) 2018-present. This file is part of V-Sekai https://v-sekai.org/.
# SaracenOne & K. S. Ernest (Fire) Lee & Lyuma & MMMaellon & Contributors
# test_qcp.gd
# SPDX-License-Identifier: MIT

extends "res://addons/gut/test.gd"

var epsilon = 0.00001

const qcp_const = preload("res://addons/many_bone_ik/qcp.gd")


func setup():
	var qcp := qcp_const.new(epsilon)
	var identity_quat := Quaternion(0, 0, 0, 1)  # Identity Quaternion

	var original := PackedVector3Array([Vector3(1, 1, 1), Vector3(2, 2, 2), Vector3(3, 3, 3)])
	var moved := PackedVector3Array()

	for vec in original:
		moved.append(identity_quat * vec)

	return [qcp, moved]


func test_weighted_superpose_with_translation():
	var setup_data = setup()
	var qcp = setup_data[0]
	var moved = setup_data[1]

	var expected: Quaternion

	var target: PackedVector3Array = moved.duplicate()
	var translation_vector := Vector3(1, 1, 1)

	for i in range(target.size()):
		target[i] = expected * (target[i] + translation_vector)

	var weight := [1.0, 1.0, 1.0]  # Equal weights
	var translate := true
	gut.p("Expected rotation: %s" % expected)
	gut.p("Initial moved: %s" % moved)
	gut.p("Expected target: %s" % target)
	var result: Quaternion = qcp.weighted_superpose(moved, target, weight, translate)
	gut.p("Result rotation: %s" % result)

	assert_almost_eq(result.x, expected.x, epsilon)
	assert_almost_eq(result.y, expected.y, epsilon)
	assert_almost_eq(result.z, expected.z, epsilon)
	assert_almost_eq(result.w, expected.w, epsilon)

	assert_eq(translate, true)
	var translation_result: Vector3 = qcp.get_translation()
	gut.p("Expected translation: %s" % translation_vector)
	gut.p("Result translation: %s" % translation_result)
	assert_almost_eq(translation_result.x, translation_vector.x, epsilon)
	assert_almost_eq(translation_result.y, translation_vector.y, epsilon)
	assert_almost_eq(translation_result.z, translation_vector.z, epsilon)


func rotate_target_headings_basis(tipHeadings: PackedVector3Array, targetHeadings: PackedVector3Array, basis: Basis):
	for i in range(tipHeadings.size()):
		targetHeadings[i] = basis * tipHeadings[i]


func test_rotation():
	var qcp := qcp_const.new(epsilon)
	var localizedTipHeadings := PackedVector3Array([
		Vector3(-14.739, -18.673, 15.040),
		Vector3(-12.473, -15.810, 16.074),
		Vector3(-14.802, -13.307, 14.408),
		Vector3(-17.782, -14.852, 16.171),
		Vector3(-16.124, -14.617, 19.584),
		Vector3(-15.029, -11.037, 18.902),
		Vector3(-18.577, -10.001, 17.996)
	])

	var localizedTargetHeadings: PackedVector3Array = localizedTipHeadings.duplicate()
	var originalTargetHeadings: PackedVector3Array = localizedTargetHeadings.duplicate()

	var basis_x := Basis(Quaternion(Vector3(1.0, 0.0, 0.0), PI / 2.0))
	var basis_y := Basis(Quaternion(Vector3(0.0, 1., 0.0), PI / 2.0))
	var basis_z := Basis(Quaternion(Vector3(0.0, 0.0, 1.), PI / 2.0))

	gut.p("Before rotation around x-axis: %s" % localizedTargetHeadings)
	rotate_target_headings_basis(localizedTipHeadings, localizedTargetHeadings, basis_x)
	gut.p("After rotation around x-axis: %s" % localizedTargetHeadings)
	assert_ne_deep(localizedTargetHeadings, originalTargetHeadings)

	gut.p("Before rotation around y-axis: %s" % localizedTargetHeadings)
	rotate_target_headings_basis(localizedTipHeadings, localizedTargetHeadings, basis_y)
	gut.p("After rotation around y-axis: %s" % localizedTargetHeadings)
	assert_ne_deep(localizedTargetHeadings, originalTargetHeadings)

	gut.p("Before rotation around z-axis: %s" % localizedTargetHeadings)
	rotate_target_headings_basis(localizedTipHeadings, localizedTargetHeadings, basis_z)
	gut.p("After rotation around z-axis: %s" % localizedTargetHeadings)
	assert_ne_deep(localizedTargetHeadings, originalTargetHeadings)
