/**************************************************************************/
/*  qcp.h                                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/math/vector3.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/variant/variant.h"

/**
 * Implementation of the Quaternion-Based Characteristic Polynomial algorithm
 * for RMSD and Superposition calculations.
 *
 * Usage:
 * 1. Create a QCP object with two Vector3 arrays of equal length as input.
 *    The input coordinates are not changed.
 * 2. Optionally, provide weighting factors [0 - 1] for each point.
 * 3. For maximum efficiency, create a QCP object once and reuse it.
 *
 * A. Calculate rmsd only: double rmsd = qcp.getRmsd();
 * B. Calculate a 4x4 transformation (Quaternion and translation) matrix: Matrix4f trans = qcp.getTransformationMatrix();
 * C. Get transformed points (y superposed onto the reference x): Vector3[] ySuperposed = qcp.getTransformedCoordinates();
 *
 * Citations:
 * - Liu P, Agrafiotis DK, & Theobald DL (2011) Reply to comment on: "Fast determination of the optimal Quaternionation matrix for macromolecular superpositions." Journal of Computational Chemistry 32(1):185-186. [http://dx.doi.org/10.1002/jcc.21606]
 * - Liu P, Agrafiotis DK, & Theobald DL (2010) "Fast determination of the optimal Quaternionation matrix for macromolecular superpositions." Journal of Computational Chemistry 31(7):1561-1563. [http://dx.doi.org/10.1002/jcc.21439]
 * - Douglas L Theobald (2005) "Rapid calculation of RMSDs using a quaternion-based characteristic polynomial." Acta Crystallogr A 61(4):478-480. [http://dx.doi.org/10.1107/S0108767305015266]
 *
 * This is an adaptation of the original C code QCPQuaternion 1.4 (2012, October 10) to C++.
 * The original C source code is available from http://theobald.brandeis.edu/qcp/ and was developed by:
 * - Douglas L. Theobald, Department of Biochemistry, Brandeis University
 * - Pu Liu, Johnson & Johnson Pharmaceutical Research and Development, L.L.C.
 *
 * @author Douglas L. Theobald (original C code)
 * @author Pu Liu (original C code)
 * @author Peter Rose (adapted to Java)
 * @author Aleix Lafita (adapted to Java)
 * @author Eron Gjoni (adapted to EWB IK)
 * @author K. S. Ernest (iFire) Lee (adapted to ManyBoneIK)
 */

class QuaternionCharacteristicPolynomial : Object {
	GDCLASS(QuaternionCharacteristicPolynomial, Object);
	double eigenvector_precision = 1E-6;

	PackedVector3Array target, moved;
	Vector<double> weight;
	double w_sum = 0;

	Vector3 target_center, moved_center;
	Quaternion rotation;
	bool translate_enabled = false;

	double sum_xy = 0, sum_xz = 0, sum_yx = 0, sum_yz = 0, sum_zx = 0, sum_zy = 0;
	double sum_xx_plus_yy = 0, sum_zz = 0, max_eigenvalue = 0, sum_yz_minus_zy = 0, sum_xz_minus_zx = 0, sum_xy_minus_yx = 0;
	double sum_xx_minus_yy = 0, sum_xy_plus_yx = 0, sum_xz_plus_zx = 0;
	double sum_yy = 0, sum_xx = 0, sum_yz_plus_zy = 0;
	bool transformation_calculated = false, inner_product_calculated = false;

	// Enhanced validation and error handling
	enum ValidationError {
		VALIDATION_OK = 0,
		ERROR_MISMATCHED_SIZES,
		ERROR_TOO_MANY_POINTS,
		ERROR_INVALID_WEIGHTS,
		ERROR_DEGENERATE_POINTS,
		ERROR_NUMERICAL_INSTABILITY
	};

	// Input validation methods
	ValidationError validate_inputs(const PackedVector3Array &p_moved, const PackedVector3Array &p_target, const Vector<double> &p_weight);
	ValidationError validate_point_sets(const PackedVector3Array &p_moved, const PackedVector3Array &p_target);
	ValidationError validate_weights(const Vector<double> &p_weight, int point_count);
	bool is_finite_vector(const Vector3 &v);
	bool are_points_degenerate(const PackedVector3Array &points);
	double calculate_point_span(const PackedVector3Array &points);

	// Enhanced single point rotation calculation
	Quaternion calculate_single_point_rotation(const Vector3 &moved_point, const Vector3 &target_point);
	
	// Quaternion canonicalization
	Quaternion apply_canonical_form(const Quaternion &q);

	// Geometric validation functions
public:
	static bool validate_rotation_normalization(const Quaternion &rotation, double tolerance = 1e-10);
	static bool validate_orthogonality(const Quaternion &rotation, double tolerance = 1e-10);
	static bool validate_point_alignment(const Quaternion &rotation, const Vector3 &translation, 
		const PackedVector3Array &moved, const PackedVector3Array &target, double tolerance = 1e-6);
	static bool validate_distance_preservation(const Quaternion &rotation, 
		const PackedVector3Array &moved, double tolerance = 1e-10);
	static double calculate_rmsd(const Quaternion &rotation, const Vector3 &translation,
		const PackedVector3Array &moved, const PackedVector3Array &target);
private:

	void inner_product(PackedVector3Array &coords1, PackedVector3Array &coords2);
	void set(PackedVector3Array &r_target, PackedVector3Array &r_moved);
	Quaternion calculate_rotation();
	void set(PackedVector3Array &p_moved, PackedVector3Array &p_target, Vector<double> &p_weight, bool p_translate);
	static void translate(Vector3 r_translate, PackedVector3Array &r_x);
	Vector3 move_to_weighted_center(PackedVector3Array &r_to_center, Vector<double> &r_weight);
	QuaternionCharacteristicPolynomial(double p_evec_prec);
	Quaternion _weighted_superpose(PackedVector3Array &p_moved, PackedVector3Array &p_target, Vector<double> &p_weight, bool translate);
	Quaternion _get_rotation();
	Vector3 _get_translation();

protected:
	static void _bind_methods();

public:
	static Array weighted_superpose(PackedVector3Array p_moved,
			PackedVector3Array p_target,
			Vector<double> p_weight, bool p_translate,
			double p_precision = 1E-6);
};
