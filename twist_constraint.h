/*************************************************************************/
/*  ik_twist_limit.h                                                     */
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

#ifndef IK_TWIST_CONSTRAINT_H
#define IK_TWIST_CONSTRAINT_H

#include "core/object/class_db.h"
#include "core/os/memory.h"
#include "core/io/resource.h"

class TwistConstraint : public Resource {
	GDCLASS(TwistConstraint, Resource);
	/**
     * Defined as some Angle in radians about the limitingAxes Y axis, 0 being equivalent to the
     * limitingAxes Z axis.
     */
	float min_twist_angle = Math_PI;
	/**
     * Defined as some Angle in radians about the limitingAxes Y axis, 0 being equivalent to the
     * minimum twist angle
     */
	float range = Math::deg2rad(540.0f);

protected:
	static void _bind_methods();

public:
	float get_min_twist_angle() const;
	void set_min_twist_angle(float p_min_axial_angle);
	float get_range() const;
	void set_range(float p_range);
	float get_min_twist_angle_degree() const;
	void set_min_twist_angle_degree(float p_min_axial_angle);
	float get_range_degree() const;
	void set_range_degree(float p_range);
	TwistConstraint();
	~TwistConstraint();
};

#endif //IK_TWIST_CONSTRAINT_H
