/*************************************************************************/
/*  qcp.cpp                                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "qcp.h"

void QCP::set_precision(double p_evec_prec, double p_eval_prec) {
	evec_prec = p_evec_prec;
	eval_prec = p_eval_prec;
}

void QCP::set_max_iterations(int32_t p_max) {
	max_iterations = p_max;
}

double QCP::calc_optimal_rotation(const PackedVector3Array &p_source, const PackedVector3Array &p_target,
		const Vector<real_t> &p_weights, Quaternion &r_quat, bool p_translate, Vector3 &r_translation) {
	double wsum = 0.0;

	PackedVector3Array source = p_source;
	PackedVector3Array target = p_target;
	Vector3 source_center;
	Vector3 target_center;
	if (p_translate) {
		wsum = center_coords(source, target, p_weights, r_translation);
	} else {
		for (int i = 0; i < p_weights.size(); i++) {
			wsum += p_weights[i];
		}
	}

	double sqrmsd = 0.0;
	// QCP doesn't handle alignment of single values, so if we only have one point
	// we just compute regular distance.
	if (p_weights.size() == 1) {
		sqrmsd = source[0].distance_squared_to(target[0]);
	} else {
		double e0 = inner_product(source, target, p_weights);
		sqrmsd = calc_sqrmsd(e0, wsum);
		r_quat = calc_rotation(e0);
	}
	return sqrmsd;
}

double QCP::center_coords(PackedVector3Array &p_source, PackedVector3Array &p_target, const Vector<real_t> &p_weights, Vector3 &translation) const {
	Vector3 c1 = Vector3();
	Vector3 c2 = Vector3();
	double wsum = 0.0;
	for (int i = 0; i < p_weights.size(); i++) {
		double w = p_weights[i];
		c1 += w * p_source[i];
		c2 += w * p_target[i];
		wsum += w;
	}
	if (!Math::is_zero_approx(wsum)) {
		c1 /= wsum;
		c2 /= wsum;
	}

	for (int i = 0; i < p_weights.size(); i++) {
		p_source.write[i] -= c1;
		p_target.write[i] -= c2;
	}

	translation = c2 - c1;

	return wsum;
}

double QCP::inner_product(const PackedVector3Array &p_source, const PackedVector3Array &p_target, const Vector<real_t> &p_weights) {
	double g1 = 0.0f;
	double g2 = 0.0f;
	double x1, x2, y1, y2, z1, z2;

	Sxx = 0;
	Sxy = 0;
	Sxz = 0;
	Syx = 0;
	Syy = 0;
	Syz = 0;
	Szx = 0;
	Szy = 0;
	Szz = 0;

	for (int i = 0; i < p_weights.size(); i++) {
		x1 = p_weights[i] * p_source[i].x;
		y1 = p_weights[i] * p_source[i].y;
		z1 = p_weights[i] * p_source[i].z;

		g1 += x1 * p_source[i].x + y1 * p_source[i].y + z1 * p_source[i].z;

		x2 = p_target[i].x;
		y2 = p_target[i].y;
		z2 = p_target[i].z;

		g2 += p_weights[i] * (x2 * x2 + y2 * y2 + z2 * z2);

		Sxx += (x1 * x2);
		Sxy += (x1 * y2);
		Sxz += (x1 * z2);

		Syx += (y1 * x2);
		Syy += (y1 * y2);
		Syz += (y1 * z2);

		Szx += (z1 * x2);
		Szy += (z1 * y2);
		Szz += (z1 * z2);
	}

	return (g1 + g2) * 0.5;
}

double QCP::calc_sqrmsd(double &e0, double wsum) {
	double Sxx2 = Sxx * Sxx;
	double Syy2 = Syy * Syy;
	double Szz2 = Szz * Szz;

	double Sxy2 = Sxy * Sxy;
	double Syz2 = Syz * Syz;
	double Sxz2 = Sxz * Sxz;

	double Syx2 = Syx * Syx;
	double Szy2 = Szy * Szy;
	double Szx2 = Szx * Szx;

	double SyzSzymSyySzz2 = 2.0 * (Syz * Szy - Syy * Szz);
	double Sxx2Syy2Szz2Syz2Szy2 = Syy2 + Szz2 - Sxx2 + Syz2 + Szy2;

	double c2 = -2.0 * (Sxx2 + Syy2 + Szz2 + Sxy2 + Syx2 + Sxz2 + Szx2 + Syz2 + Szy2);
	double c1 = 8.0 * (Sxx * Syz * Szy + Syy * Szx * Sxz + Szz * Sxy * Syx - Sxx * Syy * Szz - Syz * Szx * Sxy -
							  Szy * Syx * Sxz);

	SxzpSzx = Sxz + Szx;
	SyzpSzy = Syz + Szy;
	SxypSyx = Sxy + Syx;
	SyzmSzy = Syz - Szy;
	SxzmSzx = Sxz - Szx;
	SxymSyx = Sxy - Syx;
	SxxpSyy = Sxx + Syy;
	SxxmSyy = Sxx - Syy;

	double Sxy2Sxz2Syx2Szx2 = Sxy2 + Sxz2 - Syx2 - Szx2;

	double c0 = Sxy2Sxz2Syx2Szx2 * Sxy2Sxz2Syx2Szx2 +
				(Sxx2Syy2Szz2Syz2Szy2 + SyzSzymSyySzz2) * (Sxx2Syy2Szz2Syz2Szy2 - SyzSzymSyySzz2) +
				(-(SxzpSzx) * (SyzmSzy) + (SxymSyx) * (SxxmSyy - Szz)) * (-(SxzmSzx) * (SyzpSzy) + (SxymSyx) * (SxxmSyy + Szz)) +
				(-(SxzpSzx) * (SyzpSzy) - (SxypSyx) * (SxxpSyy - Szz)) * (-(SxzmSzx) * (SyzmSzy) - (SxypSyx) * (SxxpSyy + Szz)) +
				(+(SxypSyx) * (SyzpSzy) + (SxzpSzx) * (SxxmSyy + Szz)) * (-(SxymSyx) * (SyzmSzy) + (SxzpSzx) * (SxxpSyy + Szz)) +
				(+(SxypSyx) * (SyzmSzy) + (SxzmSzx) * (SxxmSyy - Szz)) * (-(SxymSyx) * (SyzpSzy) + (SxzmSzx) * (SxxpSyy - Szz));

	/* Newton-Raphson */
	double eignv = e0;

	int32_t i;
	for (i = 0; i < max_iterations; ++i) {
		double x2 = eignv * eignv;
		double b = (x2 + c2) * eignv;
		double a = b + c1;
		double d = (2.0 * x2 * eignv + b + a);
		if (d == 0.0) {
			break;
		}
		double delta = (a * eignv + c0) / d;
		eignv -= delta;
		if (Math::absd(delta) < Math::absd(eval_prec * eignv)) {
			break;
		}
	}

	// Printing is slow
	// if (i == max_iterations) {
	// 	WARN_PRINT(vformat("More than %d iterations needed!", max_iterations));
	// }

	double sqrmsd = Math::absd(2.0f * (e0 - eignv) / wsum);
	e0 = eignv;
	return sqrmsd;
}

Quaternion QCP::calc_rotation(double p_eigenv) const {
	double a11 = SxxpSyy + Szz - p_eigenv;
	double a12 = SyzmSzy;
	double a13 = -SxzmSzx;
	double a14 = SxymSyx;
	double a21 = SyzmSzy;
	double a22 = SxxmSyy - Szz - p_eigenv;
	double a23 = SxypSyx;
	double a24 = SxzpSzx;
	double a31 = a13;
	double a32 = a23;
	double a33 = Syy - Sxx - Szz - p_eigenv;
	double a34 = SyzpSzy;
	double a41 = a14;
	double a42 = a24;
	double a43 = a34;
	double a44 = Szz - SxxpSyy - p_eigenv;
	double a3344_4334 = a33 * a44 - a43 * a34;
	double a3244_4234 = a32 * a44 - a42 * a34;
	double a3243_4233 = a32 * a43 - a42 * a33;
	double a3143_4133 = a31 * a43 - a41 * a33;
	double a3144_4134 = a31 * a44 - a41 * a34;
	double a3142_4132 = a31 * a42 - a41 * a32;
	double q1 = a22 * a3344_4334 - a23 * a3244_4234 + a24 * a3243_4233;
	double q2 = -a21 * a3344_4334 + a23 * a3144_4134 - a24 * a3143_4133;
	double q3 = a21 * a3244_4234 - a22 * a3144_4134 + a24 * a3142_4132;
	double q4 = -a21 * a3243_4233 + a22 * a3143_4133 - a23 * a3142_4132;

	double qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

	/**
	 * The following code tries to calculate another column in the adjoint matrix when the norm of the
	 * current column is too small.
	 * Usually this block will never be activated.  To be absolutely safe this should be
	 * uncommented, but it is most likely unnecessary.
	 */
	if (qsqr < evec_prec) {
		q1 = a12 * a3344_4334 - a13 * a3244_4234 + a14 * a3243_4233;
		q2 = -a11 * a3344_4334 + a13 * a3144_4134 - a14 * a3143_4133;
		q3 = a11 * a3244_4234 - a12 * a3144_4134 + a14 * a3142_4132;
		q4 = -a11 * a3243_4233 + a12 * a3143_4133 - a13 * a3142_4132;
		qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

		if (qsqr < evec_prec) {
			double a1324_1423 = a13 * a24 - a14 * a23, a1224_1422 = a12 * a24 - a14 * a22;
			double a1223_1322 = a12 * a23 - a13 * a22, a1124_1421 = a11 * a24 - a14 * a21;
			double a1123_1321 = a11 * a23 - a13 * a21, a1122_1221 = a11 * a22 - a12 * a21;

			q1 = a42 * a1324_1423 - a43 * a1224_1422 + a44 * a1223_1322;
			q2 = -a41 * a1324_1423 + a43 * a1124_1421 - a44 * a1123_1321;
			q3 = a41 * a1224_1422 - a42 * a1124_1421 + a44 * a1122_1221;
			q4 = -a41 * a1223_1322 + a42 * a1123_1321 - a43 * a1122_1221;
			qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

			if (qsqr < evec_prec) {
				q1 = a32 * a1324_1423 - a33 * a1224_1422 + a34 * a1223_1322;
				q2 = -a31 * a1324_1423 + a33 * a1124_1421 - a34 * a1123_1321;
				q3 = a31 * a1224_1422 - a32 * a1124_1421 + a34 * a1122_1221;
				q4 = -a31 * a1223_1322 + a32 * a1123_1321 - a33 * a1122_1221;
				qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

				if (qsqr < evec_prec) {
					// /* if qsqr is still too small, return the identity matrix. */
					// WARN_PRINT("qsqr too small. Return Quaternion()");
					return Quaternion();
				}
			}
		}
	}

	// Prenormalize the result to avoid floating point errors.
	float min = q1;
	min = q2 < min ? q2 : min;
	min = q3 < min ? q3 : min;
	min = q4 < min ? q4 : min;
	return Quaternion(q2/min, q3/min, q4/min, q1/min).normalized();
}

void QCP::translate(const Vector3 p_translate, PackedVector3Array &r_source) {
	for (int i = 0; i < r_source.size(); i++) {
		r_source.write[i] += p_translate;
	}
}
