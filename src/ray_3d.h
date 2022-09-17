/*************************************************************************/
/*  ray_3d.h                                                             */
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
#ifndef ray_3d_h
#define ray_3d_h

#include "core/io/resource.h"
#include "core/math/vector3.h"

class Ray3D : public RefCounted {
	GDCLASS(Ray3D, RefCounted);

public:
	static constexpr int X = 0;
	static constexpr int Y = 1;
	static constexpr int Z = 2;

protected:
	Vector3 _p1;
	Vector3 _p2;
	Vector3 workingVector;

public:
	Vector3 tta, ttb, ttc;
	Vector3 I, u, v, n, dir, w0;
	bool inUse = false;
	Vector3 m, at, bt, ct, pt;
	Vector3 bc, ca, ac;

	virtual ~Ray3D() {}

	Ray3D();

	Ray3D(Vector3 p1, Vector3 p2);

	/**
	 * returns the point on this sgRay which is closest to the input point
	 *
	 * @param p_point
	 * @return
	 */
	virtual Vector3 closest_point_to(const Vector3 p_point);

	virtual Vector3 closest_point_to_strict(const Vector3 &p_point);

	virtual Vector3 heading();

	/**
	 * manually sets the raw variables of this
	 * ray to be equivalent to the raw variables of the
	 * target ray. Such that the two rays align without
	 * creating a new variable.
	 *
	 * @param p_target
	 */
	virtual void set_align_to(Ref<Ray3D> p_target);

	virtual void set_heading(Vector3 &p_new_head);

	/**
	 * sets the input vector equal to this sgRay's heading.
	 *
	 * @param r_set_to
	 */
	virtual Vector3 get_heading(const Vector3 &r_set_to) const;

	/**
	 * @return a copy of this ray with its z-component set to 0;
	 */
	virtual Ref<Ray3D> get_2d_copy();

	/**
	 * gets a copy of this ray, with the component specified by
	 * p_collapse_on_axis set to 0.
	 *
	 * @param p_collapse_on_axis the axis on which to collapse the ray.
	 * @return
	 */
	virtual Ref<Ray3D> get_2d_copy(const int &p_collapse_on_axis);

	virtual Vector3 get_origin();

	virtual real_t get_length();

	virtual void set_magnitude(const real_t &p_new_mag);

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
	 * 1. calling scaled_projection(someVector) should return the same value as
	 * calling
	 * scaled_projection(closestPointTo(someVector).
	 * 2. calling getMultipliedBy(scaled_projection(someVector)) should return the
	 * same
	 * vector as calling closestPointTo(someVector)
	 *
	 * @param p_input a vector to project onto this ray
	 */
	virtual real_t scaled_projection(const Vector3 &p_input);

	/**
	 * divides the ray by the amount specified by divisor, such that the
	 * base of the ray remains where it is, and the tip
	 * is scaled accordinly.
	 *
	 * @param p_divisor
	 */
	virtual void set_divide(const real_t &p_divisor);

	/**
	 * multiples the ray by the amount specified by scalar, such that the
	 * base of the ray remains where it is, and the tip
	 * is scaled accordinly.
	 *
	 * @param divisor
	 */
	virtual void set_multiply(const real_t &p_scalar);

	/**
	 * Returns a Vector3 representing where the tip
	 * of this ray would be if multiply() was called on the ray
	 * with scalar as the parameter.
	 *
	 * @param p_scalar
	 * @return
	 */
	virtual Vector3 get_multipled_by(const real_t &p_scalar);

	/**
	 * Returns a Vector3 representing where the tip
	 * of this ray would be if div() was called on the ray
	 * with scalar as the parameter.
	 *
	 * @param scalar
	 * @return
	 */
	virtual Vector3 get_divided_by(const real_t &p_divisor);

	/**
	 * Returns a Vector3 representing where the tip
	 * of this ray would be if mag(scale) was called on the ray
	 * with scalar as the parameter.
	 *
	 * @param scalar
	 * @return
	 */
	virtual Vector3 get_scaled_to(const real_t &scale);

	/**
	 * adds the specified length to the ray in both directions.
	 */
	virtual void elongate(real_t amt);

	virtual Ref<Ray3D> copy();

	virtual void reverse();

	virtual Ref<Ray3D> getReversed();

	virtual Ref<Ray3D> get_ray_scaled_to(real_t scalar);

	/*
	 * reverses this ray's direction so that it
	 * has a positive dot product with the heading of r
	 * if dot product is already positive, does nothing.
	 */
	virtual void pointWith(Ref<Ray3D> r);

	virtual void pointWith(Vector3 heading);

	virtual Ref<Ray3D> getRayScaledBy(real_t scalar);

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

	virtual void contractTo(real_t percent);

	virtual void translateTo(Vector3 newLocation);

	virtual void translateTipTo(Vector3 newLocation);

	virtual void translateBy(Vector3 toAdd);

	virtual void normalize();

	virtual Vector3 intercepts2D(Ref<Ray3D> r);

	/**
	 * If the closest point to this sgRay on the input sgRay lies
	 * beyond the bounds of that input sgRay, this returns closest point
	 * to the input Rays bound;
	 *
	 * @param r
	 * @return
	 */
	virtual Vector3 closestPointToSegment3D(Ref<Ray3D> r);

	/**
	 * returns the point on this ray which is closest to the input ray
	 *
	 * @param r
	 * @return
	 */

	virtual Vector3 closestPointToRay3D(Ref<Ray3D> r);

	virtual Vector3 closestPointToRay3DStrict(Ref<Ray3D> r);

	// returns a ray perpendicular to this ray on the XY plane;
	virtual Ref<Ray3D> getPerpendicular2D();

	/**
	 * @param ta the first vertex of a triangle on the plane
	 * @param tb the second vertex of a triangle on the plane
	 * @param tc the third vertex of a triangle on the plane
	 * @return the point where this ray intersects the plane specified by the
	 *         triangle ta,tb,tc.
	 */
	virtual Vector3 intersectsPlane(Vector3 ta, Vector3 tb, Vector3 tc);

private:
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
	virtual int intersectsSphere(Vector3 sphereCenter, real_t radius, Vector3 &S1, Vector3 &S2);

	virtual void p1(Vector3 in);

	virtual void p2(Vector3 in);

	virtual Vector3 p2();

	virtual void setP2(Vector3 p2);

	virtual Vector3 p1();

	virtual void setP1(Vector3 p1);

	virtual int intersectsSphere(Vector3 rp1, Vector3 rp2, float radius, Vector3 &S1, Vector3 &S2);

	float triArea2D(float x1, float y1, float x2, float y2, float x3, float y3) {
		return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
	}
	void barycentric(Vector3 a, Vector3 b, Vector3 c, Vector3 p, Vector3 &uvw) {
		bc = b;
		ca = a;
		at = a;
		bt = b;
		ct = c;
		pt = p;

		m = Vector3(bc - ct).cross(ca - at);

		float nu;
		float nv;
		float ood;

		float x = Math::abs(m.x);
		float y = Math::abs(m.y);
		float z = Math::abs(m.z);

		if (x >= y && x >= z) {
			nu = triArea2D(pt.y, pt.z, bt.y, bt.z, ct.y, ct.z);
			nv = triArea2D(pt.y, pt.z, ct.y, ct.z, at.y, at.z);
			ood = 1.0f / m.x;
		} else if (y >= x && y >= z) {
			nu = triArea2D(pt.x, pt.z, bt.x, bt.z, ct.x, ct.z);
			nv = triArea2D(pt.x, pt.z, ct.x, ct.z, at.x, at.z);
			ood = 1.0f / -m.y;
		} else {
			nu = triArea2D(pt.x, pt.y, bt.x, bt.y, ct.x, ct.y);
			nv = triArea2D(pt.x, pt.y, ct.x, ct.y, at.x, at.y);
			ood = 1.0f / m.z;
		}
		uvw[0] = nu * ood;
		uvw[1] = nv * ood;
		uvw[2] = 1.0f - uvw[0] - uvw[1];
	}

	virtual Vector3 planeIntersectTest(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 &uvw) {
		u = tb;
		v = tc;
		n = Vector3(0, 0, 0);
		dir = this->heading();
		w0 = Vector3(0, 0, 0);
		float r, a, b;
		u -= ta;
		v -= ta;

		n = u.cross(v);

		w0 -= ta;
		a = -(n.dot(w0));
		b = n.dot(dir);
		r = a / b;
		I = dir;
		I *= r;
		barycentric(ta, tb, tc, I, uvw);
		return I;
	}

	operator String() const {
		return String(L"(") + this->_p1.x + L" ->  " + this->_p2.x + L") \n " + L"(" + this->_p1.y + L" ->  " + this->_p2.y + L") \n " + L"(" + this->_p1.z + L" ->  " + this->_p2.z + L") \n ";
	}
};

#endif
