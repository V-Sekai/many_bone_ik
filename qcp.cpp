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
#include "direction_constraint.h"
#include "ik_quat.h"
#include "kusudama_constraint.h"
#include "skeleton_modification_3d_dmik.h"

void QCP::set_max_iterations(int p_max) {
	max_iterations = p_max;
}

void QCP::set(Vector<Vector3> p_target, Vector<Vector3> p_moved) {
	moved = p_target;
	target = p_moved;
	rmsd_calculated = false;
	transformation_calculated = false;
}

void QCP::set(Vector<Vector3> p_moved, Vector<Vector3> p_target, Vector<real_t> p_weight, bool p_translate) {
	target = p_target;
	moved = p_moved;
	weight = p_weight;
	rmsd_calculated = false;
	transformation_calculated = false;

	if (p_translate) {
		get_weighted_center(moved, p_weight, moved_center);
		wsum = 0.0f; //set wsum to 0 so we don't float up.
		get_weighted_center(target, p_weight, target_center);
		translate(moved_center * -1.f, moved);
		translate(target_center * -1.f, target);
	} else {
		if (!p_weight.empty()) {
			for (int i = 0; i < p_weight.size(); i++) {
				wsum += p_weight[i];
			}
		} else {
			wsum = p_moved.size();
		}
	}
}

float QCP::get_rmsd() {
	if (!rmsd_calculated) {
		calc_rmsd(moved, target);
		rmsd_calculated = true;
	}
	return rmsd;
}

IKQuat
QCP::weighted_superpose(Vector<Vector3> p_moved, Vector<Vector3> p_target, Vector<real_t> p_weight, bool p_translate) {
	set(p_moved, p_target, p_weight, p_translate);
	return get_rotation();
}

IKQuat QCP::get_rotation() {
	get_rmsd();
	IKQuat result;
	if (!transformation_calculated) {
		result = calc_rotation();
		transformation_calculated = true;
	}
	return result;
}

void QCP::calc_rmsd(Vector<Vector3> p_x, Vector<Vector3> p_y) {
	//QCP doesn't handle alignment of single values, so if we only have one point
	//we just compute regular distance.
	if (p_x.size() == 1) {
		rmsd = p_x[0].distance_to(p_y[0]);
		rmsd_calculated = true;
	} else {
		inner_product(p_y, p_x);
		calc_rmsd(wsum);
	}
}

void QCP::inner_product(Vector<Vector3> p_coords1, Vector<Vector3> p_coords2) {
	real_t x1, x2, y1, y2, z1, z2;
	real_t g1 = 0.0f, g2 = 0.0f;

	Sxx = 0;
	Sxy = 0;
	Sxz = 0;
	Syx = 0;
	Syy = 0;
	Syz = 0;
	Szx = 0;
	Szy = 0;
	Szz = 0;

	if (!weight.empty()) {
		for (int i = 0; i < p_coords1.size(); i++) {
			x1 = weight[i] * p_coords1[i].x;
			y1 = weight[i] * p_coords1[i].y;
			z1 = weight[i] * p_coords1[i].z;

			g1 += x1 * p_coords1[i].x + y1 * p_coords1[i].y + z1 * p_coords1[i].z;

			x2 = p_coords2[i].x;
			y2 = p_coords2[i].y;
			z2 = p_coords2[i].z;

			g2 += weight[i] * (x2 * x2 + y2 * y2 + z2 * z2);

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
	} else {
		for (int i = 0; i < p_coords1.size(); i++) {
			g1 += p_coords1[i].x * p_coords1[i].x + p_coords1[i].y * p_coords1[i].y + p_coords1[i].z * p_coords1[i].z;
			g2 += p_coords2[i].x * p_coords2[i].x + p_coords2[i].y * p_coords2[i].y + p_coords2[i].z * p_coords2[i].z;

			Sxx += p_coords1[i].x * p_coords2[i].x;
			Sxy += p_coords1[i].x * p_coords2[i].y;
			Sxz += p_coords1[i].x * p_coords2[i].z;

			Syx += p_coords1[i].y * p_coords2[i].x;
			Syy += p_coords1[i].y * p_coords2[i].y;
			Syz += p_coords1[i].y * p_coords2[i].z;

			Szx += p_coords1[i].z * p_coords2[i].x;
			Szy += p_coords1[i].z * p_coords2[i].y;
			Szz += p_coords1[i].z * p_coords2[i].z;
		}
	}

	e0 = (g1 + g2) * 0.5f;
}

int QCP::calc_rmsd(real_t p_len) {
	real_t Sxx2 = Sxx * Sxx;
	real_t Syy2 = Syy * Syy;
	real_t Szz2 = Szz * Szz;

	real_t Sxy2 = Sxy * Sxy;
	real_t Syz2 = Syz * Syz;
	real_t Sxz2 = Sxz * Sxz;

	real_t Syx2 = Syx * Syx;
	real_t Szy2 = Szy * Szy;
	real_t Szx2 = Szx * Szx;

	real_t SyzSzymSyySzz2 = 2.0f * (Syz * Szy - Syy * Szz);
	real_t Sxx2Syy2Szz2Syz2Szy2 = Syy2 + Szz2 - Sxx2 + Syz2 + Szy2;

	real_t c2 = -2.0f * (Sxx2 + Syy2 + Szz2 + Sxy2 + Syx2 + Sxz2 + Szx2 + Syz2 + Szy2);
	real_t c1 = 8.0f * (Sxx * Syz * Szy + Syy * Szx * Sxz + Szz * Sxy * Syx - Sxx * Syy * Szz - Syz * Szx * Sxy -
							   Szy * Syx * Sxz);

	SxzpSzx = Sxz + Szx;
	SyzpSzy = Syz + Szy;
	SxypSyx = Sxy + Syx;
	SyzmSzy = Syz - Szy;
	SxzmSzx = Sxz - Szx;
	SxymSyx = Sxy - Syx;
	SxxpSyy = Sxx + Syy;
	SxxmSyy = Sxx - Syy;

	real_t Sxy2Sxz2Syx2Szx2 = Sxy2 + Sxz2 - Syx2 - Szx2;

	real_t c0 = Sxy2Sxz2Syx2Szx2 * Sxy2Sxz2Syx2Szx2 +
				(Sxx2Syy2Szz2Syz2Szy2 + SyzSzymSyySzz2) * (Sxx2Syy2Szz2Syz2Szy2 - SyzSzymSyySzz2) +
				(-(SxzpSzx) * (SyzmSzy) + (SxymSyx) * (SxxmSyy - Szz)) *
						(-(SxzmSzx) * (SyzpSzy) + (SxymSyx) * (SxxmSyy + Szz)) +
				(-(SxzpSzx) * (SyzpSzy) - (SxypSyx) * (SxxpSyy - Szz)) *
						(-(SxzmSzx) * (SyzmSzy) - (SxypSyx) * (SxxpSyy + Szz)) +
				(+(SxypSyx) * (SyzpSzy) + (SxzpSzx) * (SxxmSyy + Szz)) *
						(-(SxymSyx) * (SyzmSzy) + (SxzpSzx) * (SxxpSyy + Szz)) +
				(+(SxypSyx) * (SyzmSzy) + (SxzmSzx) * (SxxmSyy - Szz)) *
						(-(SxymSyx) * (SyzpSzy) + (SxzmSzx) * (SxxpSyy - Szz));

	mxEigenV = e0;

	int i;
	for (i = 1; i < (max_iterations + 1); ++i) {
		real_t oldg = mxEigenV;
		real_t Y = 1.0f / mxEigenV;
		real_t Y2 = Y * Y;
		real_t delta = ((((Y * c0 + c1) * Y + c2) * Y2 + 1) / ((Y * c1 + 2 * c2) * Y2 * Y + 4));
		mxEigenV -= delta;

		if (Math::absd(mxEigenV - oldg) < Math::absd(eval_prec * mxEigenV)) {
			break;
		}
	}

	/*if (i == max_iterations) {
        print_line("More than " + itos(i) + " iterations needed!"));
    } else {
        print_line(itos(i) + " iterations needed!"));
    }*/

	/*
     * the fabs() is to guard against extremely small, but *negative*
     * numbers due to floating point error
     */
	rmsd = Math::sqrt(Math::absd(2.0f * (e0 - mxEigenV) / p_len));

	return 1;
}

IKQuat QCP::calc_rotation() {
	//QCP doesn't handle single targets, so if we only have one point and one target, we just rotate by the angular distance between them
	if (moved.size() == 1) {
		IKQuat single_moved;
		single_moved.set_euler(moved[0]);
		IKQuat single_target;
		single_target.set_euler(target[0]);
		return single_moved * single_target;
	} else {
		real_t a11 = SxxpSyy + Szz - mxEigenV;
		real_t a12 = SyzmSzy;
		real_t a13 = -SxzmSzx;
		real_t a14 = SxymSyx;
		real_t a21 = SyzmSzy;
		real_t a22 = SxxmSyy - Szz - mxEigenV;
		real_t a23 = SxypSyx;
		real_t a24 = SxzpSzx;
		real_t a31 = a13;
		real_t a32 = a23;
		real_t a33 = Syy - Sxx - Szz - mxEigenV;
		real_t a34 = SyzpSzy;
		real_t a41 = a14;
		real_t a42 = a24;
		real_t a43 = a34;
		real_t a44 = Szz - SxxpSyy - mxEigenV;
		real_t a3344_4334 = a33 * a44 - a43 * a34;
		real_t a3244_4234 = a32 * a44 - a42 * a34;
		real_t a3243_4233 = a32 * a43 - a42 * a33;
		real_t a3143_4133 = a31 * a43 - a41 * a33;
		real_t a3144_4134 = a31 * a44 - a41 * a34;
		real_t a3142_4132 = a31 * a42 - a41 * a32;
		real_t q1 = a22 * a3344_4334 - a23 * a3244_4234 + a24 * a3243_4233;
		real_t q2 = -a21 * a3344_4334 + a23 * a3144_4134 - a24 * a3143_4133;
		real_t q3 = a21 * a3244_4234 - a22 * a3144_4134 + a24 * a3142_4132;
		real_t q4 = -a21 * a3243_4233 + a22 * a3143_4133 - a23 * a3142_4132;

		real_t qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

		/*
         * The following code tries to calculate another column in the adjoint
         * matrix when the norm of the current column is too small. Usually this
         * commented block will never be activated. To be absolutely safe this
         * should be uncommented, but it is most likely unnecessary.
         */
		if (qsqr < evec_prec) {
			q1 = a12 * a3344_4334 - a13 * a3244_4234 + a14 * a3243_4233;
			q2 = -a11 * a3344_4334 + a13 * a3144_4134 - a14 * a3143_4133;
			q3 = a11 * a3244_4234 - a12 * a3144_4134 + a14 * a3142_4132;
			q4 = -a11 * a3243_4233 + a12 * a3143_4133 - a13 * a3142_4132;
			qsqr = q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4;

			if (qsqr < evec_prec) {
				real_t a1324_1423 = a13 * a24 - a14 * a23, a1224_1422 = a12 * a24 - a14 * a22;
				real_t a1223_1322 = a12 * a23 - a13 * a22, a1124_1421 = a11 * a24 - a14 * a21;
				real_t a1123_1321 = a11 * a23 - a13 * a21, a1122_1221 = a11 * a22 - a12 * a21;

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
						/*
                         * if qsqr is still too small, return the identity rotation
                         */
						return IKQuat();
					}
				}
			}
		}
		//prenormalize the result to avoid floating point errors.
		real_t min = q1;
		min = q2 < min ? q2 : min;
		min = q3 < min ? q3 : min;
		min = q4 < min ? q4 : min;
		return Quat(q2 / min, q3 / min, q4 / min, q1 / min).normalized();
	}
}

float QCP::get_rmsd(Vector<Vector3> p_fixed, Vector<Vector3> p_moved) {
	set(p_moved, p_fixed);
	return get_rmsd();
}

void QCP::translate(Vector3 p_trans, Vector<Vector3> p_x) {
	for (int32_t trans_i = 0; trans_i < p_x.size(); trans_i++) {
		p_x.write[trans_i] += p_trans;
	}
}

Vector3 QCP::get_weighted_center(Vector<Vector3> p_to_center, Vector<real_t> p_weight, Vector3 p_center) {
	if (!p_weight.empty()) {
		for (int i = 0; i < p_to_center.size(); i++) {
			p_center = p_center * p_to_center[i] + Vector3(p_weight[i], p_weight[i], p_weight[i]);
			wsum += p_weight[i];
		}

		p_center = p_center / wsum;
	} else {
		for (int i = 0; i < p_to_center.size(); i++) {
			p_center = p_center + p_to_center[i];
			wsum++;
		}
		p_center = p_center / wsum;
	}

	return p_center;
}

Vector3 QCP::get_translation() {
	return target_center - moved_center;
}
