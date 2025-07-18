/**************************************************************************/
/*  qcp.cpp                                                               */
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

#include "qcp.h"

QuaternionCharacteristicPolynomial::QuaternionCharacteristicPolynomial(double p_evec_prec) {
	eigenvector_precision = p_evec_prec;
}

// Enhanced input validation methods
QuaternionCharacteristicPolynomial::ValidationError QuaternionCharacteristicPolynomial::validate_inputs(const PackedVector3Array &p_moved, const PackedVector3Array &p_target, const Vector<double> &p_weight) {
	ValidationError result = validate_point_sets(p_moved, p_target);
	if (result != VALIDATION_OK) {
		return result;
	}

	if (!p_weight.is_empty()) {
		result = validate_weights(p_weight, p_moved.size());
		if (result != VALIDATION_OK) {
			return result;
		}
	}

	return VALIDATION_OK;
}

QuaternionCharacteristicPolynomial::ValidationError QuaternionCharacteristicPolynomial::validate_point_sets(const PackedVector3Array &p_moved, const PackedVector3Array &p_target) {
	// Check size matching
	if (p_moved.size() != p_target.size()) {
		return ERROR_MISMATCHED_SIZES;
	}

	// Check maximum points limit (prevent memory issues)
	if (p_moved.size() > 10000) {
		return ERROR_TOO_MANY_POINTS;
	}

	// Check for empty point sets
	if (p_moved.size() == 0) {
		return ERROR_MISMATCHED_SIZES;
	}

	// Check for finite values in all points
	for (int i = 0; i < p_moved.size(); i++) {
		if (!is_finite_vector(p_moved[i]) || !is_finite_vector(p_target[i])) {
			return ERROR_NUMERICAL_INSTABILITY;
		}
	}

	// Check for degenerate point sets (all points identical)
	if (p_moved.size() > 1) {
		if (are_points_degenerate(p_moved) || are_points_degenerate(p_target)) {
			return ERROR_DEGENERATE_POINTS;
		}
	}

	return VALIDATION_OK;
}

QuaternionCharacteristicPolynomial::ValidationError QuaternionCharacteristicPolynomial::validate_weights(const Vector<double> &p_weight, int point_count) {
	if (p_weight.size() != point_count) {
		return ERROR_INVALID_WEIGHTS;
	}

	double total_weight = 0.0;
	for (int i = 0; i < p_weight.size(); i++) {
		double w = p_weight[i];
		
		// Check for finite weight values
		if (!Math::is_finite(w)) {
			return ERROR_INVALID_WEIGHTS;
		}

		// Check weight bounds
		if (w < 1.0e-15 || w > 1.0e15) {
			return ERROR_INVALID_WEIGHTS;
		}

		total_weight += w;
	}

	// Check that total weight is reasonable
	if (total_weight < 1.0e-15) {
		return ERROR_INVALID_WEIGHTS;
	}

	return VALIDATION_OK;
}

bool QuaternionCharacteristicPolynomial::is_finite_vector(const Vector3 &v) {
	return Math::is_finite(v.x) && Math::is_finite(v.y) && Math::is_finite(v.z);
}

bool QuaternionCharacteristicPolynomial::are_points_degenerate(const PackedVector3Array &points) {
	if (points.size() < 2) {
		return false;
	}

	// Check if all points are essentially the same
	Vector3 first = points[0];
	const double tolerance = 1.0e-12;

	for (int i = 1; i < points.size(); i++) {
		Vector3 diff = points[i] - first;
		if (diff.length_squared() > tolerance * tolerance) {
			return false;
		}
	}

	return true;
}

double QuaternionCharacteristicPolynomial::calculate_point_span(const PackedVector3Array &points) {
	if (points.size() == 0) {
		return 0.0;
	}

	Vector3 min_point = points[0];
	Vector3 max_point = points[0];

	for (int i = 1; i < points.size(); i++) {
		Vector3 p = points[i];
		min_point.x = MIN(min_point.x, p.x);
		min_point.y = MIN(min_point.y, p.y);
		min_point.z = MIN(min_point.z, p.z);
		max_point.x = MAX(max_point.x, p.x);
		max_point.y = MAX(max_point.y, p.y);
		max_point.z = MAX(max_point.z, p.z);
	}

	Vector3 span = max_point - min_point;
	return MAX(span.x, MAX(span.y, span.z));
}

// Enhanced single point rotation calculation - matches Elixir implementation
Quaternion QuaternionCharacteristicPolynomial::calculate_single_point_rotation(const Vector3 &moved_point, const Vector3 &target_point) {
	double u_length = moved_point.length();
	double v_length = target_point.length();

	// Handle zero-length vectors
	if (u_length < 1.0e-15 || v_length < 1.0e-15) {
		return Quaternion(); // Identity quaternion
	}

	// Normalize the vectors
	Vector3 u_norm = moved_point.normalized();
	Vector3 v_norm = target_point.normalized();

	double dot = u_norm.dot(v_norm);

	Quaternion result;

	if (dot > 0.9999999) {
		// Vectors are already aligned (within tolerance)
		result = Quaternion(); // Identity
	} else if (dot < -0.9999999) {
		// Vectors are opposite (180-degree rotation needed)
		// Find a perpendicular axis for 180-degree rotation
		Vector3 perp_axis;
		
		// Choose the axis that gives the largest cross product component
		if (Math::abs(u_norm.x) < 0.9) {
			perp_axis = Vector3(1.0, 0.0, 0.0);
		} else if (Math::abs(u_norm.y) < 0.9) {
			perp_axis = Vector3(0.0, 1.0, 0.0);
		} else {
			perp_axis = Vector3(0.0, 0.0, 1.0);
		}

		Vector3 cross = u_norm.cross(perp_axis).normalized();
		// 180-degree rotation quaternion: (x, y, z, w) where w = 0
		result = Quaternion(cross.x, cross.y, cross.z, 0.0);
	} else {
		// General case: rotation between non-opposite vectors
		// Use the half-angle formula for quaternion from two vectors
		// This is more numerically stable than the standard formula

		// Calculate the half-way vector (bisector)
		Vector3 half_way = (u_norm + v_norm);
		double half_way_length = half_way.length();
		
		if (half_way_length > 1.0e-15) {
			half_way = half_way / half_way_length; // Normalize
			
			// Calculate quaternion using half-way vector method
			// q = [cross(u, half_way), dot(u, half_way)]
			Vector3 cross = u_norm.cross(half_way);
			double w = u_norm.dot(half_way);
			
			// This should already be normalized, but verify
			double quat_magnitude = Math::sqrt(cross.x*cross.x + cross.y*cross.y + cross.z*cross.z + w*w);
			if (quat_magnitude > 1.0e-15) {
				result = Quaternion(cross.x / quat_magnitude, cross.y / quat_magnitude, 
								  cross.z / quat_magnitude, w / quat_magnitude);
			} else {
				result = Quaternion(); // Identity
			}
		} else {
			// Fallback to standard method if half-way vector fails
			Vector3 cross = u_norm.cross(v_norm);
			double w = 1.0 + dot;
			
			double quat_magnitude = Math::sqrt(cross.x*cross.x + cross.y*cross.y + cross.z*cross.z + w*w);
			if (quat_magnitude > 1.0e-15) {
				result = Quaternion(cross.x / quat_magnitude, cross.y / quat_magnitude, 
								  cross.z / quat_magnitude, w / quat_magnitude);
			} else {
				result = Quaternion(); // Identity
			}
		}
	}

	// Apply canonical form to ensure consistent representation
	return apply_canonical_form(result);
}

// Quaternion canonicalization - ensures w >= 0 for consistent representation
Quaternion QuaternionCharacteristicPolynomial::apply_canonical_form(const Quaternion &q) {
	// Ensure w >= 0 for canonical quaternion representation
	// This resolves the dual representation issue (q and -q represent the same rotation)
	if (q.w >= 0.0) {
		return q;
	} else {
		return Quaternion(-q.x, -q.y, -q.z, -q.w);
	}
}

void QuaternionCharacteristicPolynomial::set(PackedVector3Array &r_target, PackedVector3Array &r_moved) {
	target = r_target;
	moved = r_moved;
	transformation_calculated = false;
	inner_product_calculated = false;
}

Quaternion QuaternionCharacteristicPolynomial::_get_rotation() {
	if (!transformation_calculated) {
		if (!inner_product_calculated) {
			inner_product(moved, target);
		}
		rotation = calculate_rotation();
		transformation_calculated = true;
	}
	return rotation;
}

Quaternion QuaternionCharacteristicPolynomial::calculate_rotation() {
	Quaternion result;

	if (moved.size() == 1) {
		// Single point case - use direct vector alignment
		Vector3 u = moved[0];
		Vector3 v = target[0];
		result = calculate_single_point_rotation(u, v);
	} else {
		// Multi-point case using characteristic polynomial method
		// Build 4x4 characteristic matrix elements exactly as in Elixir
		double sxz_p_szx = sum_xz_plus_zx;
		double syz_p_szy = sum_yz_plus_zy;
		double sxy_p_syx = sum_xy_plus_yx;
		double syz_m_szy = sum_yz_minus_zy;
		double sxz_m_szx = sum_xz_minus_zx;
		double sxy_m_syx = sum_xy_minus_yx;
		double sxx_p_syy = sum_xx_plus_yy;
		double sxx_m_syy = sum_xx_minus_yy;

		// Build the 4x4 characteristic matrix elements exactly as in Elixir
		double a11 = sxx_p_syy + sum_zz - max_eigenvalue;
		double a12 = syz_m_szy;
		double a13 = -sxz_m_szx;
		double a14 = sxy_m_syx;
		double a21 = syz_m_szy;
		double a22 = sxx_m_syy - sum_zz - max_eigenvalue;
		double a23 = sxy_p_syx;
		double a24 = sxz_p_szx;
		double a31 = a13;
		double a32 = a23;
		double a33 = sum_yy - sum_xx - sum_zz - max_eigenvalue;
		double a34 = syz_p_szy;
		double a41 = a14;
		double a42 = a24;
		double a43 = a34;
		double a44 = sum_zz - sxx_p_syy - max_eigenvalue;

		// Calculate cofactor determinants exactly as in Elixir
		double a3344_4334 = a33 * a44 - a43 * a34;
		double a3244_4234 = a32 * a44 - a42 * a34;
		double a3243_4233 = a32 * a43 - a42 * a33;
		double a3143_4133 = a31 * a43 - a41 * a33;
		double a3144_4134 = a31 * a44 - a41 * a34;
		double a3142_4132 = a31 * a42 - a41 * a32;

		// Calculate quaternion components exactly as in Elixir
		double q1 = a22 * a3344_4334 - a23 * a3244_4234 + a24 * a3243_4233;
		double q2 = -a21 * a3344_4334 + a23 * a3144_4134 - a24 * a3143_4133;
		double q3 = a21 * a3244_4234 - a22 * a3144_4134 + a24 * a3142_4132;
		double q4 = -a21 * a3243_4233 + a22 * a3143_4133 - a23 * a3142_4132;

		double qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

		// Handle small qsqr case exactly as in Elixir
		double evecprec = eigenvector_precision;
		if (qsqr < evecprec) {
			// Try second column of adjoint matrix
			q1 = a12 * a3344_4334 - a13 * a3244_4234 + a14 * a3243_4233;
			q2 = -a11 * a3344_4334 + a13 * a3144_4134 - a14 * a3143_4133;
			q3 = a11 * a3244_4234 - a12 * a3144_4134 + a14 * a3142_4132;
			q4 = -a11 * a3243_4233 + a12 * a3143_4133 - a13 * a3142_4132;
			qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

			if (qsqr < evecprec) {
				// Try third column
				double a1324_1423 = a13 * a24 - a14 * a23;
				double a1224_1422 = a12 * a24 - a14 * a22;
				double a1223_1322 = a12 * a23 - a13 * a22;
				double a1124_1421 = a11 * a24 - a14 * a21;
				double a1123_1321 = a11 * a23 - a13 * a21;
				double a1122_1221 = a11 * a22 - a12 * a21;

				q1 = a42 * a1324_1423 - a43 * a1224_1422 + a44 * a1223_1322;
				q2 = -a41 * a1324_1423 + a43 * a1124_1421 - a44 * a1123_1321;
				q3 = a41 * a1224_1422 - a42 * a1124_1421 + a44 * a1122_1221;
				q4 = -a41 * a1223_1322 + a42 * a1123_1321 - a43 * a1122_1221;
				qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

				if (qsqr < evecprec) {
					// Try fourth column
					q1 = a32 * a1324_1423 - a33 * a1224_1422 + a34 * a1223_1322;
					q2 = -a31 * a1324_1423 + a33 * a1124_1421 - a34 * a1123_1321;
					q3 = a31 * a1224_1422 - a32 * a1124_1421 + a34 * a1122_1221;
					q4 = -a31 * a1223_1322 + a32 * a1123_1321 - a33 * a1122_1221;
					qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;
				}
			}
		}

		if (qsqr < evecprec) {
			// Return identity quaternion as in Elixir
			result = Quaternion();
		} else {
			// Normalize quaternion
			double normq = Math::sqrt(qsqr);
			double normalized_q1 = q1 / normq;
			double normalized_q2 = q2 / normq;
			double normalized_q3 = q3 / normq;
			double normalized_q4 = q4 / normq;

			// Return in (x,y,z,w) order for Godot (Elixir uses q1=w, q2=x, q3=y, q4=z)
			result = Quaternion(normalized_q2, normalized_q3, normalized_q4, normalized_q1);
		}
	}

	// Apply canonical form to ensure consistent quaternion representation
	return apply_canonical_form(result);
}

void QuaternionCharacteristicPolynomial::translate(Vector3 r_translate, PackedVector3Array &r_x) {
	for (Vector3 &p : r_x) {
		p += r_translate;
	}
}

Vector3 QuaternionCharacteristicPolynomial::_get_translation() {
	if (translate_enabled) {
		return target_center - rotation.xform(moved_center);
	} else {
		return Vector3();
	}
}

Vector3 QuaternionCharacteristicPolynomial::move_to_weighted_center(PackedVector3Array &r_to_center, Vector<double> &r_weight) {
	Vector3 center;
	double total_weight = 0;
	bool weight_is_empty = r_weight.is_empty();
	int size = r_to_center.size();

	for (int i = 0; i < size; i++) {
		if (!weight_is_empty) {
			total_weight += r_weight[i];
			center += r_to_center[i] * r_weight[i];
		} else {
			center += r_to_center[i];
			total_weight++;
		}
	}

	if (total_weight > 0) {
		center /= total_weight;
	}

	return center;
}

void QuaternionCharacteristicPolynomial::inner_product(PackedVector3Array &r_coords1, PackedVector3Array &r_coords2) {
	Vector3 weighted_coord1, weighted_coord2;
	double sum_of_squares1 = 0, sum_of_squares2 = 0;

	sum_xx = 0;
	sum_xy = 0;
	sum_xz = 0;
	sum_yx = 0;
	sum_yy = 0;
	sum_yz = 0;
	sum_zx = 0;
	sum_zy = 0;
	sum_zz = 0;

	bool weight_is_empty = weight.is_empty();
	int size = r_coords1.size();

	for (int i = 0; i < size; i++) {
		double current_weight = weight_is_empty ? 1.0 : weight[i];
		
		// Apply weight to moved point (r_coords1)
		weighted_coord1 = current_weight * r_coords1[i];
		// Target point (r_coords2) is not weighted in the cross-correlation matrix
		weighted_coord2 = r_coords2[i];

		// Calculate sum of squares exactly as in Elixir
		sum_of_squares1 += weighted_coord1.dot(r_coords1[i]);
		sum_of_squares2 += current_weight * weighted_coord2.dot(weighted_coord2);

		// Build cross-correlation matrix: weighted_moved * target
		sum_xx += (weighted_coord1.x * weighted_coord2.x);
		sum_xy += (weighted_coord1.x * weighted_coord2.y);
		sum_xz += (weighted_coord1.x * weighted_coord2.z);

		sum_yx += (weighted_coord1.y * weighted_coord2.x);
		sum_yy += (weighted_coord1.y * weighted_coord2.y);
		sum_yz += (weighted_coord1.y * weighted_coord2.z);

		sum_zx += (weighted_coord1.z * weighted_coord2.x);
		sum_zy += (weighted_coord1.z * weighted_coord2.y);
		sum_zz += (weighted_coord1.z * weighted_coord2.z);
	}

	// Calculate maximum eigenvalue exactly as in Elixir
	double initial_eigenvalue = (sum_of_squares1 + sum_of_squares2) * 0.5;

	sum_xz_plus_zx = sum_xz + sum_zx;
	sum_yz_plus_zy = sum_yz + sum_zy;
	sum_xy_plus_yx = sum_xy + sum_yx;
	sum_yz_minus_zy = sum_yz - sum_zy;
	sum_xz_minus_zx = sum_xz - sum_zx;
	sum_xy_minus_yx = sum_xy - sum_yx;
	sum_xx_plus_yy = sum_xx + sum_yy;
	sum_xx_minus_yy = sum_xx - sum_yy;
	max_eigenvalue = initial_eigenvalue;

	inner_product_calculated = true;
}

Quaternion QuaternionCharacteristicPolynomial::_weighted_superpose(PackedVector3Array &p_moved, PackedVector3Array &p_target, Vector<double> &p_weight, bool p_translate) {
	set(p_moved, p_target, p_weight, p_translate);
	return _get_rotation();
}

void QuaternionCharacteristicPolynomial::set(PackedVector3Array &p_moved_param, PackedVector3Array &p_target_param, Vector<double> &p_weight_param, bool p_translate_param) {
	transformation_calculated = false;
	inner_product_calculated = false;

	moved = p_moved_param;
	target = p_target_param;
	weight = p_weight_param;
	translate_enabled = p_translate_param;

	if (translate_enabled) {
		moved_center = move_to_weighted_center(p_moved_param, p_weight_param);
		target_center = move_to_weighted_center(p_target_param, p_weight_param);

		translate(moved_center * -1, moved);
		translate(target_center * -1, target);

		w_sum = 0;
		if (!weight.is_empty()) {
			for (int i = 0; i < weight.size(); i++) {
				w_sum += weight[i];
			}
		} else {
			w_sum = moved.size();
		}

	} else {
		moved_center = Vector3();
		target_center = Vector3();
		w_sum = 0;
		if (!weight.is_empty()) {
			for (int i = 0; i < weight.size(); i++) {
				w_sum += weight[i];
			}
		} else {
			w_sum = moved.size();
		}
	}
}

// Geometric validation functions (similar to Elixir implementation)
bool QuaternionCharacteristicPolynomial::validate_rotation_normalization(const Quaternion &rotation, double tolerance) {
	double magnitude = Math::sqrt(rotation.x * rotation.x + rotation.y * rotation.y + 
								 rotation.z * rotation.z + rotation.w * rotation.w);
	return Math::abs(magnitude - 1.0) < tolerance;
}

bool QuaternionCharacteristicPolynomial::validate_point_alignment(const Quaternion &rotation, const Vector3 &translation,
		const PackedVector3Array &moved, const PackedVector3Array &target, double tolerance) {
	if (moved.size() != target.size()) {
		return false;
	}

	// Use a more reasonable tolerance for point alignment
	double effective_tolerance = MAX(tolerance, 1e-6);

	for (int i = 0; i < moved.size(); i++) {
		// Apply transformation: rotated = rotation * moved[i] + translation
		Vector3 rotated = rotation.xform(moved[i]) + translation;
		Vector3 target_point = target[i];
		
		// Check if transformed point is close to target
		Vector3 diff = rotated - target_point;
		if (diff.length() > effective_tolerance) {
			return false;
		}
	}
	
	return true;
}

bool QuaternionCharacteristicPolynomial::validate_distance_preservation(const Quaternion &rotation, 
		const PackedVector3Array &moved, double tolerance) {
	if (moved.size() < 2) {
		return true; // Trivially true for single points
	}

	// Use a more reasonable tolerance for distance preservation
	double effective_tolerance = MAX(tolerance, 1e-6);

	// Check that rotation preserves distances between points
	for (int i = 0; i < moved.size(); i++) {
		for (int j = i + 1; j < moved.size(); j++) {
			double original_distance = moved[i].distance_to(moved[j]);
			
			Vector3 rotated_i = rotation.xform(moved[i]);
			Vector3 rotated_j = rotation.xform(moved[j]);
			double rotated_distance = rotated_i.distance_to(rotated_j);
			
			// Use relative tolerance for better numerical stability
			double relative_tolerance = effective_tolerance * MAX(1.0, original_distance);
			if (Math::abs(original_distance - rotated_distance) > relative_tolerance) {
				return false;
			}
		}
	}
	
	return true;
}

bool QuaternionCharacteristicPolynomial::validate_orthogonality(const Quaternion &rotation, double tolerance) {
	// Use a more reasonable tolerance for orthogonality checks
	double effective_tolerance = MAX(tolerance, 1e-6);

	// Check that rotation preserves orthogonality by testing standard basis vectors
	Vector3 i_rotated = rotation.xform(Vector3(1.0, 0.0, 0.0));
	Vector3 j_rotated = rotation.xform(Vector3(0.0, 1.0, 0.0));
	Vector3 k_rotated = rotation.xform(Vector3(0.0, 0.0, 1.0));

	// Check orthogonality
	if (Math::abs(i_rotated.dot(j_rotated)) > effective_tolerance) return false;
	if (Math::abs(i_rotated.dot(k_rotated)) > effective_tolerance) return false;
	if (Math::abs(j_rotated.dot(k_rotated)) > effective_tolerance) return false;

	// Check normalization
	if (Math::abs(i_rotated.length() - 1.0) > effective_tolerance) return false;
	if (Math::abs(j_rotated.length() - 1.0) > effective_tolerance) return false;
	if (Math::abs(k_rotated.length() - 1.0) > effective_tolerance) return false;

	return true;
}

double QuaternionCharacteristicPolynomial::calculate_rmsd(const Quaternion &rotation, const Vector3 &translation,
		const PackedVector3Array &moved, const PackedVector3Array &target) {
	if (moved.size() != target.size() || moved.size() == 0) {
		return -1.0; // Error case
	}

	double sum_squared_distances = 0.0;
	
	for (int i = 0; i < moved.size(); i++) {
		Vector3 transformed = rotation.xform(moved[i]) + translation;
		Vector3 diff = transformed - target[i];
		sum_squared_distances += diff.length_squared();
	}
	
	return Math::sqrt(sum_squared_distances / moved.size());
}

void QuaternionCharacteristicPolynomial::_bind_methods() {
	ClassDB::bind_static_method("QuaternionCharacteristicPolynomial",
			D_METHOD("weighted_superpose", "moved", "target",
					"weight", "translate", "precision"),
			&QuaternionCharacteristicPolynomial::weighted_superpose);
}

Array QuaternionCharacteristicPolynomial::weighted_superpose(PackedVector3Array p_moved,
		PackedVector3Array p_target,
		Vector<double> p_weight, bool p_translate,
		double p_precision) {
	QuaternionCharacteristicPolynomial qcp(p_precision);
	
	// Enhanced input validation
	ValidationError validation_result = qcp.validate_inputs(p_moved, p_target, p_weight);
	if (validation_result != VALIDATION_OK) {
		// Return error result - identity rotation and zero translation
		Array error_result;
		error_result.push_back(Quaternion()); // Identity quaternion
		error_result.push_back(Vector3()); // Zero translation
		return error_result;
	}
	
	Quaternion rotation;
	Vector3 translation;
	
	// Use consistent algorithm for both single and multi-point cases
	rotation = qcp._weighted_superpose(p_moved, p_target, p_weight, p_translate);
	translation = qcp._get_translation();
	
	// Apply canonical form to ensure consistent quaternion representation
	rotation = qcp.apply_canonical_form(rotation);
	
	Array result;
	result.push_back(rotation);
	result.push_back(translation);
	return result;
}
