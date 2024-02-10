# Copyright (c) 2018-present. This file is part of V-Sekai https://v-sekai.org/.
# SaracenOne & K. S. Ernest (Fire) Lee & Lyuma & MMMaellon & Contributors
# test_qcp.gd
# SPDX-License-Identifier: MIT

# Copyright (c) 2018-present. This file is part of V-Sekai https://v-sekai.org/.
# SaracenOne & K. S. Ernest (Fire) Lee & Lyuma & MMMaellon & Contributors
# test_qcp.gd
# SPDX-License-Identifier: MIT

# Implementation of the Quaternion-Based Characteristic Polynomial algorithm
# for RMSD and Superposition calculations.
#
# Usage:
# 1. Create a QCP object with two Vector3 arrays of equal length as input.
#    The input coordinates are not changed.
# 2. Optionally, provide weighting factors [0 - 1] for each point.
# 3. For maximum efficiency, create a QCP object once and reuse it.
#
# A. Calculate rmsd only: double rmsd = qcp.getRmsd();
# B. Calculate a 4x4 transformation (Quaternion and translation) matrix: Matrix4f trans = qcp.getTransformationMatrix();
# C. Get transformed points (y superposed onto the reference x): Vector3[] ySuperposed = qcp.getTransformedCoordinates();
#
# Citations:
# - Liu P, Agrafiotis DK, & Theobald DL (2011) Reply to comment on: "Fast determination of the optimal quaternionation matrix for macromolecular superpositions." Journal of Computational Chemistry 32(1):185-186. [http://dx.doi.org/10.1002/jcc.21606]
# - Liu P, Agrafiotis DK, & Theobald DL (2010) "Fast determination of the optimal quaternionation matrix for macromolecular superpositions." Journal of Computational Chemistry 31(7):1561-1563. [http://dx.doi.org/10.1002/jcc.21439]
# - Douglas L Theobald (2005) "Rapid calculation of RMSDs using a quaternion-based characteristic polynomial." Acta Crystallogr A 61(4):478-480. [http://dx.doi.org/10.1107/S0108767305015266]
#
# This is an adaptation of the original C code QCPQuaternion 1.4 (2012, October 10) to C++.
# The original C source code is available from http://theobald.brandeis.edu/qcp/ and was developed by:
# - Douglas L. Theobald, Department of Biochemistry, Brandeis University
# - Pu Liu, Johnson & Johnson Pharmaceutical Research and Development, L.L.C.
#
# Authors:
# - Douglas L. Theobald (original C code)
# - Pu Liu (original C code)
# - Peter Rose (adapted to Java)
# - Aleix Lafita (adapted to Java)
# - Eron Gjoni (adapted to EWB IK)
# - K. S. Ernest (iFire) Lee (adapted to ManyBoneIK)


extends "res://addons/gut/test.gd"

var epsilon = 0.00001

const qcp_const = preload("res://addons/many_bone_ik_script/qcp.gd")


func test_weighted_superpose():
	var qcp := qcp_const.new(epsilon)
	var expected := Quaternion(0, 0, sqrt(2) / 2, sqrt(2) / 2)
	var moved := PackedVector3Array([Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)])
	var target := moved.duplicate()

	for i in range(target.size()):
		target[i] = expected * target[i]

	var weight := [1.0, 1.0, 1.0]  # Equal weights
	var result := qcp.weighted_superpose(moved, target, weight, false)

	assert_almost_eq(result.x, expected.x, epsilon)
	assert_almost_eq(result.y, expected.y, epsilon)
	assert_almost_eq(result.z, expected.z, epsilon)
	assert_almost_eq(result.w, expected.w, epsilon)


func test_weighted_translation():
	var qcp := qcp_const.new(epsilon)
	var expected := Quaternion()
	var moved := PackedVector3Array([Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)])
	var target := moved.duplicate()
	var translation_vector := Vector3(1, 2, 3)

	for i in range(target.size()):
		target[i] = expected * (target[i] + translation_vector)

	var weight := [1.0, 1.0, 1.0]  # Equal weights
	var translate := true
	var result := qcp.weighted_superpose(moved, target, weight, translate)

	# Quaternion checks
	assert_almost_eq(result.x, expected.x, epsilon)
	assert_almost_eq(result.y, expected.y, epsilon)
	assert_almost_eq(result.z, expected.z, epsilon)
	assert_almost_eq(result.w, expected.w, epsilon)

	# Translation checks
	assert_eq(translate, true)
	var inverted_expected := expected.inverse()
	var translation_result := inverted_expected * qcp.get_translation()
	assert_almost_eq(translation_result.x, translation_vector.x, epsilon)
	assert_almost_eq(translation_result.y, translation_vector.y, epsilon)
	assert_almost_eq(translation_result.z, translation_vector.z, epsilon)


func test_weighted_translation_shortest_path():
	var qcp := qcp_const.new(epsilon)
	var expected := Quaternion(1, 2, 3, 4).normalized()
	var moved := PackedVector3Array([Vector3(4, 5, 6), Vector3(7, 8, 9), Vector3(1, 2, 3)])
	var target := moved.duplicate()
	var translation_vector := Vector3(1, 2, 3)

	for i in range(target.size()):
		target[i] = expected * (target[i] + translation_vector)

	var weight := [1.0, 1.0, 1.0]  # Equal weights
	var translate := true
	var result := qcp.weighted_superpose(moved, target, weight, translate)

	# Quaternion checks
	assert_almost_eq(result.x, expected.x, epsilon)
	assert_almost_eq(result.y, expected.y, epsilon)
	assert_almost_eq(result.z, expected.z, epsilon)
	assert_almost_eq(result.w, expected.w, epsilon)

	# Translation checks
	assert_eq(translate, true)

	var inverted_expected := expected.inverse()
	var translation_result := inverted_expected * qcp.get_translation()
	assert_almost_ne(translation_result.x, translation_vector.x, epsilon)
	assert_almost_ne(translation_result.y, translation_vector.y, epsilon)
	assert_almost_ne(translation_result.z, translation_vector.z, epsilon)
