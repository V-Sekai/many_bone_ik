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

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

class Ray3D : public CanLoad {
public:
	static constexpr int X = 0;
	static constexpr int Y = 1;
	static constexpr int Z = 2;

protected:
	Vector3 p1_Conflict;
	Vector3 p2_Conflict;
	Vector3 workingVector;

public:
	Vector3 tta, *ttb, *ttc;
	Vector3 I, *u, *v, *n, *dir, *w0;
	bool inUse = false;
	Vector3 m, *at, *bt, *ct, *pt;
	Vector3 bc, *ca, *ac;

	virtual ~Ray3D() {
		delete p1_Conflict;
		delete p2_Conflict;
		delete workingVector;
		delete tta;
		delete ttb;
		delete ttc;
		delete I;
		delete u;
		delete v;
		delete n;
		delete dir;
		delete w0;
		delete m;
		delete at;
		delete bt;
		delete ct;
		delete pt;
		delete bc;
		delete ca;
		delete ac;
	}

	Ray3D();

	Ray3D(Vector3 origin);

	Ray3D(Vector3 p1, Vector3 p2);

	static float triArea2D(float x1, float y1, float x2, float y2, float x3, float y3);

	virtual float distTo(Vector3 point);

	/**
	 * returns the distance between the input point and the point on this ray
	 * (treated as a lineSegment) to which the input is closest.
	 *
	 * @param point
	 * @return
	 */
	virtual float distToStrict(Vector3 point);

	/**
	 * returns the distance between this ray treated as a line and the input ray
	 * treated as a line.
	 *
	 * @param r
	 * @return
	 */
	virtual float distTo(ewbik::math::Ray3D *r);

	/**
	 * returns the distance between this ray as a line segment, and the input ray
	 * treated as a line segment
	 */
	virtual float distToStrict(ewbik::math::Ray3D *r);

	/**
	 * returns the point on this sgRay which is closest to the input point
	 *
	 * @param point
	 * @return
	 */
	virtual Vector3 closestPointTo(Vector3 point);

	virtual Vector3 closestPointToStrict(Vector3 point);

	virtual Vector3 heading();

	/**
	 * manually sets the raw variables of this
	 * ray to be equivalent to the raw variables of the
	 * target ray. Such that the two rays align without
	 * creating a new variable.
	 *
	 * @param target
	 */
	virtual void alignTo(ewbik::math::Ray3D *target);

	virtual void heading(std::vector<float> &newHead);

	virtual void heading(Vector3 newHead);

	/**
	 * sets the input vector equal to this sgRay's heading.
	 *
	 * @param setTo
	 */
	virtual void getHeading(Vector3 setTo);

	/**
	 * @return a copy of this ray with its z-component set to 0;
	 */
	virtual ewbik::math::Ray3D *get2DCopy();

	/**
	 * gets a copy of this ray, with the component specified by
	 * collapseOnAxis set to 0.
	 *
	 * @param collapseOnAxis the axis on which to collapse the ray.
	 * @return
	 */
	virtual ewbik::math::Ray3D *get2DCopy(int collapseOnAxis);

	virtual Vector3 origin();

	virtual float mag();

	virtual void mag(float newMag);

	/**
	 * Returns the scalar projection of the input vector on this
	 * ray. In other words, if this ray goes from (5, 0) to (10, 0),
	 * and the input vector is (7.5, 7), this function
	 * would output 0.5. Because that is amount the ray would need
	 * to be scaled by so that its tip is where the vector would project onto
	 * this ray.
	 * <p>
	 * Due to floating point errors, the intended properties of this function might
	 * not be entirely consistent with its output under summation.
	 * <p>
	 * To help spare programmer cognitive cycles debugging in such circumstances,
	 * the intended properties
	 * are listed for reference here (despite their being easily inferred).
	 * <p>
	 * 1. calling scaledProjection(someVector) should return the same value as
	 * calling
	 * scaledProjection(closestPointTo(someVector).
	 * 2. calling getMultipliedBy(scaledProjection(someVector)) should return the
	 * same
	 * vector as calling closestPointTo(someVector)
	 *
	 * @param input a vector to project onto this ray
	 */
	virtual float scaledProjection(Vector3 input);

	/**
	 * divides the ray by the amount specified by divisor, such that the
	 * base of the ray remains where it is, and the tip
	 * is scaled accordinly.
	 *
	 * @param divisor
	 */
	virtual void div(float divisor);

	/**
	 * multiples the ray by the amount specified by scalar, such that the
	 * base of the ray remains where it is, and the tip
	 * is scaled accordinly.
	 *
	 * @param divisor
	 */
	virtual void multiply(float scalar);

	/**
	 * Returns a Vector3 representing where the tip
	 * of this ray would be if multiply() was called on the ray
	 * with scalar as the parameter.
	 *
	 * @param scalar
	 * @return
	 */
	virtual Vector3 getMultipledBy(float scalar);

	/**
	 * Returns a Vector3 representing where the tip
	 * of this ray would be if div() was called on the ray
	 * with scalar as the parameter.
	 *
	 * @param scalar
	 * @return
	 */
	virtual Vector3 getDivideddBy(float divisor);

	/**
	 * Returns a Vector3 representing where the tip
	 * of this ray would be if mag(scale) was called on the ray
	 * with scalar as the parameter.
	 *
	 * @param scalar
	 * @return
	 */
	virtual Vector3 getScaledTo(float scale);

	/**
	 * adds the specified length to the ray in both directions.
	 */
	virtual void elongate(float amt);

	virtual ewbik::math::Ray3D *copy();

	virtual void reverse();

	virtual ewbik::math::Ray3D *getReversed();

	virtual ewbik::math::Ray3D *getRayScaledTo(float scalar);

	/*
	 * reverses this ray's direction so that it
	 * has a positive dot product with the heading of r
	 * if dot product is already positive, does nothing.
	 */
	virtual void pointWith(ewbik::math::Ray3D *r);

	virtual void pointWith(Vector3 heading);

	virtual ewbik::math::Ray3D *getRayScaledBy(float scalar);

	/**
	 * sets the values of the given vector to where the
	 * tip of this Ray would be if the ray were inverted
	 *
	 * @param vec
	 * @return the vector that was passed in after modification (for chaining)
	 */
	virtual Vector3 setToInvertedTip(Vector3 vec);

	/*
	 * public Vector3 closestPointToSegment3DStrict(sgRay r) {
	 *
	 * }
	 */

	virtual void contractTo(float percent);

	virtual void translateTo(Vector3 newLocation);

	virtual void translateTipTo(Vector3 newLocation);

	virtual void translateBy(Vector3 toAdd);

	virtual void normalize();

	virtual Vector3 intercepts2D(ewbik::math::Ray3D *r);

	/**
	 * If the closest point to this sgRay on the input sgRay lies
	 * beyond the bounds of that input sgRay, this returns closest point
	 * to the input Rays bound;
	 *
	 * @param r
	 * @return
	 */
	virtual Vector3 closestPointToSegment3D(ewbik::math::Ray3D *r);

	/**
	 * returns the point on this ray which is closest to the input ray
	 *
	 * @param r
	 * @return
	 */

	virtual Vector3 closestPointToRay3D(ewbik::math::Ray3D *r);

	virtual Vector3 closestPointToRay3DStrict(ewbik::math::Ray3D *r);

	/**
	 * returns the point on this ray which is closest to
	 * the input sgRay. If that point lies outside of the bounds
	 * of this ray, returns null.
	 *
	 * @param r
	 * @return
	 */
	virtual Vector3 closestPointToRay3DBounded(ewbik::math::Ray3D *r);

	// returns a ray perpendicular to this ray on the XY plane;
	virtual ewbik::math::Ray3D *getPerpendicular2D();

	virtual Vector3 intercepts2DStrict(ewbik::math::Ray3D *r);

	/**
	 * @param ta the first vertex of a triangle on the plane
	 * @param tb the second vertex of a triangle on the plane
	 * @param tc the third vertex of a triangle on the plane
	 * @return the point where this ray intersects the plane specified by the
	 *         triangle ta,tb,tc.
	 */
	virtual Vector3 intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc);

	virtual Vector3 intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc, std::vector<float> &uvw);

	/**
	 * @param ta     the first vertex of a triangle on the plane
	 * @param tb     the second vertex of a triangle on the plane
	 * @param tc     the third vertex of a triangle on the plane
	 * @param result the variable in which to hold the result
	 */
	virtual void intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 result);

	/**
	 * Similar to intersectsPlane, but returns false if intersection does not occur
	 * on the triangle strictly defined by ta, tb, and tc
	 *
	 * @param ta     the first vertex of a triangle on the plane
	 * @param tb     the second vertex of a triangle on the plane
	 * @param tc     the third vertex of a triangle on the plane
	 * @param result the variable in which to hold the result
	 */
	virtual bool intersectsTriangle(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 result);

private:
	Vector3 planeIntersectTest(Vector3 ta, Vector3 tb, Vector3 tc, std::vector<float> &uvw);

	/*
	 * Find where this ray intersects a sphere
	 *
	 * @param Vector3 the center of the sphere to test against.
	 *
	 * @param radius radius of the sphere
	 *
	 * @param S1 reference to variable in which the first intersection will be
	 * placed
	 *
	 * @param S2 reference to variable in which the second intersection will be
	 * placed
	 *
	 * @return number of intersections found;
	 */
public:
	virtual int intersectsSphere(Vector3 sphereCenter, float radius, Vector3 S1, Vector3 S2);

	/*
	 * Find where this ray intersects a sphere
	 *
	 * @param radius radius of the sphere
	 *
	 * @param S1 reference to variable in which the first intersection will be
	 * placed
	 *
	 * @param S2 reference to variable in which the second intersection will be
	 * placed
	 *
	 * @return number of intersections found;
	 */
	virtual int intersectsSphere(Vector3 rp1, Vector3 rp2, float radius, Vector3 S1, Vector3 S2);

	virtual void barycentric(Vector3 a, Vector3 b, Vector3 c, Vector3 p, std::vector<float> &uvw);

	virtual std::wstring toString();

	virtual void p1(Vector3 in);

	virtual void p2(Vector3 in);

	virtual float lerp(float a, float b, float t);

	virtual Vector3 p2();

	virtual void set(Ray3D *r);

	virtual void setP2(Vector3 p2);

	virtual Vector3 p1();

	virtual void setP1(Vector3 p1);
};