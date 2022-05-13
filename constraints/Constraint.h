/*************************************************************************/
/*  test_ewbik.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#pragma once

#include "core/io/resource.h"

class Constraint : public Resource {
	GDCLASS(Constraint, Resource);

public:
	virtual void snapToLimits() = 0;
	virtual void disable() = 0;
	virtual void enable() = 0;
	virtual bool isEnabled() = 0;

	// The constraint returns true if the ray from the constraint origin to the globalPoint is within the constraint's limits
	// and false otherwise.
	virtual bool isInLimits_(Vector3 globalPoint) = 0;
	virtual Transform3D limitingAxes() = 0;

	/**
	 * @return a measure of the rotational freedom afforded by this constraint.
	 * with 0 meaning no rotational freedom (the bone is essentially stationary in relation to its parent)
	 * and 1 meaning full rotational freedom (the bone is completely unconstrained).
	 *
	 * This should be computed as ratio between orientations a bone can be in and orientations that
	 * a bone cannot be in as defined by its representation as a point on the surface of a hypersphere.
	 */
	virtual double getRotationalFreedom() = 0;
};
