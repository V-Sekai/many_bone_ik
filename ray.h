/*************************************************************************/
/*  ray.h                                                                */
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

#ifndef GODOT_ANIMATION_UNIFIED_BEZIERS_RAY_H
#define GODOT_ANIMATION_UNIFIED_BEZIERS_RAY_H

#include "core/math/vector3.h"

struct Ray {
	Vector3 position = Vector3();
	Vector3 normal = Vector3();

	/**
     * sets the values of the given vector to where the
     * tip of this Ray would be if the ray were inverted
     * 
     * @return the vector that was passed in after modification (for chaining)
     */
	Vector3 set_to_inverted_tip();

	Ray();

	Ray(Vector3 p_position, Vector3 p_normal);

	/**
     *  adds the specified length to the ray in both directions.
     */
	void elongate(float p_amount);

	/* Find where this ray intersects a sphere
        * @param SGVec_3d the center of the sphere to test against.
        * @param radius radius of the sphere
        * @param S1 reference to variable in which the first intersection will be placed
        * @param S2 reference to variable in which the second intersection will be placed
        * @return number of intersections found;
        */
	int intersects_sphere(Vector3 sphereCenter, double radius, Vector3 S1, Vector3 S2);

	/* Find where this ray intersects a sphere
        * @param radius radius of the sphere
        * @param S1 reference to variable in which the first intersection will be placed
        * @param S2 reference to variable in which the second intersection will be placed
        * @return number of intersections found;
        */
	int intersects_sphere(Vector3 rp1, Vector3 rp2, double radius, Vector3 &S1, Vector3 &S2);
};

#endif //GODOT_ANIMATION_UNIFIED_BEZIERS_RAY_H
