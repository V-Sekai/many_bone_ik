# Copyright (c) 2018-present. This file is part of V-Sekai https://v-sekai.org/.
# K. S. Ernest (Fire) Lee & Contributors
# test_qcp.gd
# SPDX-License-Identifier: MIT

@tool
extends GutTest

var QCP = preload("res://many_bone_ik/core/qcp.gd")


## Method to test weighted superpose
func test_weighted_superpose():

	var expected: Quaternion = Quaternion(0, 0, sqrt(2) / 2, sqrt(2) / 2)
	var moved: Array[Vector3] = [Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)]
	var target: Array[Vector3] = moved.duplicate()
	for i in range(target.size()):
		target[i] = expected * target[i]

	gut.p("Moved: %s" % str(moved))
	gut.p("Target: %s" %  str(target))
	var weight: Array[float] = [1.0, 1.0, 1.0] # Equal weights

	var epsilon = 1e-6
	var qcp = QCP.new(epsilon)
	var result: Quaternion = qcp.weighted_superpose(moved, target, weight, false)
	gut.p("Expected: %s" % expected)
	gut.p("Result: %s" % result)
	assert_lt(abs(result.x - expected.x), epsilon)
	assert_lt(abs(result.y - expected.y), epsilon)
	assert_lt(abs(result.z - expected.z), epsilon)
	assert_lt(abs(result.w - expected.w), epsilon)


## Method to test weighted translation
func test_weighted_translation():
	var epsilon = 1e-6
	var qcp = QCP.new(epsilon)

	var expected = Quaternion()
	var moved = [Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)]
	var target = moved.duplicate()
	var translation_vector = Vector3(1, 2, 3)
	for i in range(target.size()):
		target[i] = expected * (target[i] + translation_vector)

	var weight = [1.0, 1.0, 1.0] # Equal weights
	var translate = true
	gut.p("Moved: %s" % [moved])
	gut.p("Target: %s" % [target])
	gut.p("Weight: %s" % [weight])
	var result = qcp.weighted_superpose(moved, target, weight, translate)
	assert_lt(abs(result.x - expected.x), epsilon)
	assert_lt(abs(result.y - expected.y), epsilon)
	assert_lt(abs(result.z - expected.z), epsilon)
	assert_lt(abs(result.w - expected.w), epsilon)

	# Check if translation occurred
	assert_true(translate)
	var translation_result = expected.inverse() * qcp.get_translation()
	assert_lt(abs(translation_result.x - translation_vector.x), epsilon)
	assert_lt(abs(translation_result.y - translation_vector.y), epsilon)
	assert_lt(abs(translation_result.z - translation_vector.z), epsilon)


## Method to test weighted translation shortest path
func test_weighted_translation_shortest_path():
	var epsilon = 1e-6
	var qcp = QCP.new(epsilon)

	var expected = Quaternion(1, 2, 3, 4).normalized()
	var moved = [Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)]
	var target = moved.duplicate()
	var translation_vector = Vector3(1, 2, 3)
	for i in range(target.size()):
		target[i] = expected * (target[i] + translation_vector)

	var weight = [1.0, 1.0, 1.0] # Equal weights
	var translate = true

	var result = qcp.weighted_superpose(moved, target, weight, translate)
	assert_true(abs(result.x - expected.x) > epsilon)
	assert_true(abs(result.y - expected.y) > epsilon)
	assert_true(abs(result.z - expected.z) > epsilon)
	assert_true(abs(result.w - expected.w) > epsilon)

	# Check if translation occurred
	assert_true(translate)
	var translation_result = expected.inverse() * qcp.get_translation()
	assert_true(abs(translation_result.x - translation_vector.x) > epsilon)
	assert_true(abs(translation_result.y - translation_vector.y) > epsilon)
	assert_true(abs(translation_result.z - translation_vector.z) > epsilon)
