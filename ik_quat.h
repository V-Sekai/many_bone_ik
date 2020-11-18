/*************************************************************************/
/*  ik_quat.h                                                            */
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

#ifndef GODOT_ANIMATION_UNIFIED_BEZIERS_IK_QUAT_H
#define GODOT_ANIMATION_UNIFIED_BEZIERS_IK_QUAT_H

#include "core/math/quat.h"
#include "core/templates/vector.h"

class IKQuat : public Quat {
public:
	Vector<IKQuat> get_swing_twist(Vector3 p_axis);

	void clamp_to_quadrance_angle(float p_cos_half_angle);

	void clamp_to_angle(float p_angle);

	inline IKQuat(float p_x, float p_y, float p_z, float p_w) :
			Quat(p_x,
					p_y,
					p_z,
					p_w) {
	}

	IKQuat(Quat p_quat) {
		x = p_quat.x;
		y = p_quat.y;
		z = p_quat.z;
		w = p_quat.w;
	}

	IKQuat() {
	}

	~IKQuat() {
	}
};

#endif //GODOT_ANIMATION_UNIFIED_BEZIERS_IK_QUAT_H
