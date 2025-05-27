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

void QuaternionCharacteristicPolynomial::set(PackedVector3Array &r_target, PackedVector3Array &r_moved) {
	target = r_target;
	moved = r_moved;
	transformation_calculated = false;
	inner_product_calculated = false;
}

Quaternion QuaternionCharacteristicPolynomial::_get_rotation() {
	if (!transformation_calculated) {
		if (!inner_product_calculated) {
			inner_product(target, moved);
		}
		rotation = calculate_rotation();
		transformation_calculated = true;
	}
	return rotation;
}

Quaternion QuaternionCharacteristicPolynomial::calculate_rotation() {
	Quaternion result;

	if (moved.size() == 1) {
		Vector3 u = moved[0];
		Vector3 v = target[0];
		double norm_product = u.length() * v.length();

		if (norm_product == 0.0) {
			return Quaternion();
		}

		double dot = u.dot(v);

		if (dot < ((2.0e-15 - 1.0) * norm_product)) {
			Vector3 w = u.normalized();
			result = Quaternion(w.x, w.y, w.z, 0.0f).normalized();
		} else {
			double q0 = Math::sqrt(0.5 * (1.0 + dot / norm_product));
			double coeff = 1.0 / (2.0 * q0 * norm_product);
			Vector3 q = v.cross(u).normalized();
			result = Quaternion(coeff * q.x, coeff * q.y, coeff * q.z, q0).normalized();
		}
	} else {
		double a13 = -sum_xz_minus_zx;
		double a14 = sum_xy_minus_yx;
		double a21 = sum_yz_minus_zy;
		double a22 = sum_xx_minus_yy - sum_zz - max_eigenvalue;
		double a23 = sum_xy_plus_yx;
		double a24 = sum_xz_plus_zx;
		double a31 = a13;
		double a32 = a23;
		double a33 = sum_yy - sum_xx - sum_zz - max_eigenvalue;
		double a34 = sum_yz_plus_zy;
		double a41 = a14;
		double a42 = a24;
		double a43 = a34;
		double a44 = sum_zz - sum_xx_plus_yy - max_eigenvalue;

		double a3344_4334 = a33 * a44 - a43 * a34;
		double a3244_4234 = a32 * a44 - a42 * a34;
		double a3243_4233 = a32 * a43 - a42 * a33;
		double a3143_4133 = a31 * a43 - a41 * a33;
		double a3144_4134 = a31 * a44 - a41 * a34;
		double a3142_4132 = a31 * a42 - a41 * a32;

		double quaternion_w = a22 * a3344_4334 - a23 * a3244_4234 + a24 * a3243_4233;
		double quaternion_x = a21 * a3344_4334 - a23 * a3144_4134 + a24 * a3143_4133;
		double quaternion_y = -a21 * a3244_4234 + a22 * a3144_4134 - a24 * a3142_4132;
		double quaternion_z = a21 * a3243_4233 - a22 * a3143_4133 + a23 * a3142_4132;

		double min_comp_val = quaternion_w;
		min_comp_val = quaternion_x < min_comp_val ? quaternion_x : min_comp_val;
		min_comp_val = quaternion_y < min_comp_val ? quaternion_y : min_comp_val;
		min_comp_val = quaternion_z < min_comp_val ? quaternion_z : min_comp_val;

		if (!Math::is_zero_approx(min_comp_val)) {
			quaternion_w /= min_comp_val;
			quaternion_x /= min_comp_val;
			quaternion_y /= min_comp_val;
			quaternion_z /= min_comp_val;
		}

		double qsqr = quaternion_w * quaternion_w + quaternion_x * quaternion_x + quaternion_y * quaternion_y + quaternion_z * quaternion_z;

		if (qsqr < eigenvector_precision) {
			result = Quaternion();
		} else {
			result = Quaternion(quaternion_x, quaternion_y, quaternion_z, quaternion_w).normalized();
		}
	}

	return result;
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
		if (!weight_is_empty) {
			weighted_coord1 = weight[i] * r_coords1[i];
			sum_of_squares1 += weighted_coord1.dot(r_coords1[i]);
		} else {
			weighted_coord1 = r_coords1[i];
			sum_of_squares1 += weighted_coord1.dot(weighted_coord1);
		}

		weighted_coord2 = r_coords2[i];

		sum_of_squares2 += weight_is_empty ? weighted_coord2.dot(weighted_coord2) : (weight[i] * weighted_coord2.dot(weighted_coord2));

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
	Quaternion rotation =
			qcp._weighted_superpose(p_moved, p_target, p_weight, p_translate);
	Vector3 translation = qcp._get_translation();
	Array result;
	result.push_back(rotation);
	result.push_back(translation);
	return result;
}
