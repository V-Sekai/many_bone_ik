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
	real_t evec_prec = FLT_EPSILON;
	real_t eval_prec = CMP_EPSILON;
	int32_t max_iterations = 15;
	real_t Sxx, Sxy, Sxz, Syx, Syy, Syz, Szx, Szy, Szz = 0;
	real_t SxxpSyy, SyzmSzy, SxzmSzx, SxymSyx = 0;
	real_t SxxmSyy, SxypSyx, SxzpSzx, SyzpSzy = 0;

	real_t inner_product(const PackedVector3Array &p_coords1, const PackedVector3Array &p_coords2, const Vector<real_t> &p_weights);
	real_t center_coords(PackedVector3Array &p_coords1, PackedVector3Array &p_coords2, const Vector<real_t> &p_weights, Vector3 &translation) const;
	real_t calc_sqrmsd(real_t &e0, real_t wsum);
	Quaternion calc_rotation(real_t p_eigenv) const;

public:
	void set_precision(real_t p_evec_prec, real_t p_eval_prec);
	void set_max_iterations(int32_t p_max);
	real_t calc_optimal_rotation(const PackedVector3Array &p_coords1, const PackedVector3Array &p_coords2,
			const Vector<real_t> &p_weights, Quaternion &p_quat);
};

#endif // QCP_H