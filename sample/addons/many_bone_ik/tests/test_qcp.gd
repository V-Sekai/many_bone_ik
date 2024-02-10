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
	var rotation_axis := Vector3(1, 0, 0)  # Rotation axis (example: around x-axis)
	var rotation_angle := PI / 4  # Rotation angle (example: 45 degrees)
	identity_quat = Quaternion(rotation_axis, rotation_angle)

	var original := PackedVector3Array([Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)])
	var moved := PackedVector3Array()

	for vec in original:
		moved.append(identity_quat * vec)

	return [qcp, moved]


func test_weighted_superpose_with_translation():
	var setup_data = setup()
	var qcp = setup_data[0]
	var moved = setup_data[1]
	var expected := Quaternion()
	var target: PackedVector3Array = moved.duplicate()
	var translation_vector := Vector3(1, 2, 3)

	for i in range(target.size()):
		target[i] = expected * (target[i] + translation_vector)

	var weight := [1.0, 1.0, 1.0]  # Equal weights
	var translate := true
	gut.p("Expected rotation: %s" % expected)
	var result: Quaternion = qcp.weighted_superpose(moved, target, weight, translate)
	gut.p("Result rotation: %s" % result)
	# Quaternion checks
	assert_almost_eq(result.x, expected.x, epsilon)
	assert_almost_eq(result.y, expected.y, epsilon)
	assert_almost_eq(result.z, expected.z, epsilon)
	assert_almost_eq(result.w, expected.w, epsilon)

	# Translation checks
	assert_eq(translate, true)
	var inverted_expected := expected.inverse()
	gut.p("Expected translation: %s" % translation_vector)
	var translation_result: Vector3 = inverted_expected * qcp.get_translation()
	gut.p("Result translation: %s" % translation_result)
	assert_almost_eq(translation_result.x, translation_vector.x, epsilon)
	assert_almost_eq(translation_result.y, translation_vector.y, epsilon)
	assert_almost_eq(translation_result.z, translation_vector.z, epsilon)
