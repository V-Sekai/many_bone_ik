
#ifndef IK_RAY_3D_H
#define IK_RAY_3D_H

#include "core/io/resource.h"
#include "core/math/vector3.h"

class IKRay3D : public RefCounted {
	GDCLASS(IKRay3D, RefCounted);

	Vector3 tta, ttb, ttc;
	Vector3 I, u, v, n, dir, w0;
	Vector3 m, at, bt, ct, pt;
	Vector3 bc, ca, ac;

	Vector3 point_1;
	Vector3 point_2;
	Vector3 working_vector;

protected:
	static void _bind_methods();

public:
	IKRay3D();
	virtual ~IKRay3D() {}
	IKRay3D(Vector3 p_point_one, Vector3 p_point_two);
	virtual Vector3 get_heading();
	virtual void set_heading(const Vector3 &p_new_head);

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
	 * 1. calling get_scaled_projection(someVector) should return the same value as
	 * calling
	 * get_scaled_projection(closestPointTo(someVector).
	 * 2. calling getMultipliedBy(get_scaled_projection(someVector)) should return the
	 * same
	 * vector as calling closestPointTo(someVector)
	 *
	 * @param p_input a vector to project onto this ray
	 */
	virtual real_t get_scaled_projection(const Vector3 p_input);

	/**
	 * adds the specified length to the ray in both directions.
	 */
	virtual void elongate(real_t p_amount);

	/**
	 * @param ta the first vertex of a triangle on the plane
	 * @param tb the second vertex of a triangle on the plane
	 * @param tc the third vertex of a triangle on the plane
	 * @return the point where this ray intersects the plane specified by the
	 *         triangle ta,tb,tc.
	 */
	virtual Vector3 get_intersects_plane(Vector3 p_vertex_a, Vector3 p_vertex_b, Vector3 p_vertex_c);

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
	virtual int intersects_sphere(Vector3 p_sphere_center, real_t p_radius, Vector3 *r_first_intersection, Vector3 *r_second_intersection);
	virtual void p1(Vector3 p_point);
	virtual void p2(Vector3 p_point);
	virtual Vector3 p2();
	virtual Vector3 p1();
	virtual int intersects_sphere(Vector3 p_rp1, Vector3 p_rp2, float p_radius, Vector3 *r_first_intersection, Vector3 *r_second_intersection);
	float triangle_area_2d(float p_x1, float p_y1, float p_x2, float p_y2, float p_x3, float p_y3);
	void barycentric(Vector3 p_a, Vector3 p_b, Vector3 p_c, Vector3 p_p, Vector3 *r_uvw);
	virtual Vector3 plane_intersect_test(Vector3 p_vertex_a, Vector3 p_vertex_b, Vector3 p_vertex_c, Vector3 *uvw);
	operator String() const {
		return String(L"(") + point_1.x + L" ->  " + point_2.x + L") \n " + L"(" + point_1.y + L" ->  " + point_2.y + L") \n " + L"(" + point_1.z + L" ->  " + point_2.z + L") \n ";
	}
};

#endif // IK_RAY_3D_H


IKRay3D::IKRay3D() {
}

IKRay3D::IKRay3D(Vector3 p_p1, Vector3 p_p2) {
	working_vector = p_p1;
	point_1 = p_p1;
	point_2 = p_p2;
}

Vector3 IKRay3D::get_heading() {
	working_vector = point_2;
	return working_vector - point_1;
}

void IKRay3D::set_heading(const Vector3 &p_new_head) {
	point_2 = point_1;
	point_2 = p_new_head;
}

real_t IKRay3D::get_scaled_projection(const Vector3 p_input) {
	working_vector = p_input;
	working_vector = working_vector - point_1;
	Vector3 heading = get_heading();
	real_t headingMag = heading.length();
	real_t workingVectorMag = working_vector.length();
	if (workingVectorMag == 0 || headingMag == 0) {
		return 0;
	}
	return (working_vector.dot(heading) / (headingMag * workingVectorMag)) * (workingVectorMag / headingMag);
}

void IKRay3D::elongate(real_t amt) {
	Vector3 midPoint = (point_1 + point_2) * 0.5f;
	Vector3 p1Heading = point_1 - midPoint;
	Vector3 p2Heading = point_2 - midPoint;
	Vector3 p1Add = p1Heading.normalized() * amt;
	Vector3 p2Add = p2Heading.normalized() * amt;

	point_1 = p1Heading + p1Add + midPoint;
	point_2 = p2Heading + p2Add + midPoint;
}

Vector3 IKRay3D::get_intersects_plane(Vector3 ta, Vector3 tb, Vector3 tc) {
	Vector3 uvw;
	tta = ta;
	ttb = tb;
	ttc = tc;
	tta -= point_1;
	ttb -= point_1;
	ttc -= point_1;
	Vector3 result = plane_intersect_test(tta, ttb, ttc, &uvw);
	return result + point_1;
}

int IKRay3D::intersects_sphere(Vector3 sphereCenter, real_t radius, Vector3 *S1, Vector3 *S2) {
	Vector3 tp1 = point_1 - sphereCenter;
	Vector3 tp2 = point_2 - sphereCenter;
	int result = intersects_sphere(tp1, tp2, radius, S1, S2);
	*S1 += sphereCenter;
	*S2 += sphereCenter;
	return result;
}

void IKRay3D::p1(Vector3 in) {
	point_1 = in;
}

void IKRay3D::p2(Vector3 in) {
	point_2 = in;
}

Vector3 IKRay3D::p2() {
	return point_2;
}

Vector3 IKRay3D::p1() {
	return point_1;
}

int IKRay3D::intersects_sphere(Vector3 rp1, Vector3 rp2, float radius, Vector3 *S1, Vector3 *S2) {
	Vector3 direction = rp2 - rp1;
	Vector3 e = direction; // e=ray.dir
	e.normalize(); // e=g/|g|
	Vector3 h = point_1;
	h = Vector3(0.0f, 0.0f, 0.0f);
	h = h - rp1; // h=r.o-c.M
	float lf = e.dot(h); // lf=e.h
	float radpow = radius * radius;
	float hdh = h.length_squared();
	float lfpow = lf * lf;
	float s = radpow - hdh + lfpow; // s=r^2-h^2+lf^2
	if (s < 0.0f) {
		return 0; // no intersection points ?
	}
	s = Math::sqrt(s); // s=sqrt(r^2-h^2+lf^2)

	int result = 0;
	if (lf < s) {
		if (lf + s >= 0) {
			s = -s; // swap S1 <-> S2}
			result = 1; // one intersection point
		}
	} else {
		result = 2; // 2 intersection points
	}

	*S1 = e * (lf - s);
	*S1 += rp1; // S1=A+e*(lf-s)
	*S2 = e * (lf + s);
	*S2 += rp1; // S2=A+e*(lf+s)
	return result;
}

Vector3 IKRay3D::plane_intersect_test(Vector3 ta, Vector3 tb, Vector3 tc, Vector3 *uvw) {
	u = tb;
	v = tc;
	n = Vector3(0, 0, 0);
	dir = get_heading();
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

float IKRay3D::triangle_area_2d(float x1, float y1, float x2, float y2, float x3, float y3) {
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
}

void IKRay3D::barycentric(Vector3 a, Vector3 b, Vector3 c, Vector3 p, Vector3 *uvw) {
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
		nu = triangle_area_2d(pt.y, pt.z, bt.y, bt.z, ct.y, ct.z);
		nv = triangle_area_2d(pt.y, pt.z, ct.y, ct.z, at.y, at.z);
		ood = 1.0f / m.x;
	} else if (y >= x && y >= z) {
		nu = triangle_area_2d(pt.x, pt.z, bt.x, bt.z, ct.x, ct.z);
		nv = triangle_area_2d(pt.x, pt.z, ct.x, ct.z, at.x, at.z);
		ood = 1.0f / -m.y;
	} else {
		nu = triangle_area_2d(pt.x, pt.y, bt.x, bt.y, ct.x, ct.y);
		nv = triangle_area_2d(pt.x, pt.y, ct.x, ct.y, at.x, at.y);
		ood = 1.0f / m.z;
	}
	(*uvw)[0] = nu * ood;
	(*uvw)[1] = nv * ood;
	(*uvw)[2] = 1.0f - (*uvw)[0] - (*uvw)[1];
}

void IKRay3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_heading"), &IKRay3D::get_heading);
	ClassDB::bind_method(D_METHOD("get_scaled_projection", "input"), &IKRay3D::get_scaled_projection);
	ClassDB::bind_method(D_METHOD("get_intersects_plane", "a", "b", "c"), &IKRay3D::get_intersects_plane);
}
