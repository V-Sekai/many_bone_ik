/*************************************************************************/
/*  qcp.h                                                                */
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

#ifndef QCP_H
#define QCP_H

#include "core/math/quaternion.h"
#include "core/variant/variant.h"

class QCP {
private:
	double evec_prec = CMP_EPSILON2;
	double eval_prec = CMP_EPSILON2;
	int32_t max_iterations = 30;
	double Sxx, Sxy, Sxz, Syx, Syy, Syz, Szx, Szy, Szz = 0;
	double SxxpSyy, SyzmSzy, SxzmSzx, SxymSyx = 0;
	double SxxmSyy, SxypSyx, SxzpSzx, SyzpSzy = 0;

	double inner_product(const PackedVector3Array &p_source, const PackedVector3Array &p_target, const Vector<real_t> &p_weights);
	double center_coords(PackedVector3Array &r_source, PackedVector3Array &r_target, const Vector<real_t> &p_weights, Vector3 &translation) const;
	double calc_sqrmsd(double &e0, double wsum);
	Quaternion calc_rotation(double p_eigenv) const;
	void translate(const Vector3 p_translate, PackedVector3Array &r_source);

public:
	void set_precision(double p_evec_prec, double p_eval_prec);
	void set_max_iterations(int32_t p_max);
	double calc_optimal_rotation(const PackedVector3Array &p_source, const PackedVector3Array &p_target,
			const Vector<real_t> &p_weights, Quaternion &r_quat, bool p_translate, Vector3 &r_translation);
};

#endif // QCP_H