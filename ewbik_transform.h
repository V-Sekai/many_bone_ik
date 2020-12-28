#pragma once
#include "core/math/quat.h"
#include "core/math/transform.h"
#include "core/math/vector3.h"
#include "ray.h"

class EWBIKTransform {
private:
	Vector3 x_base = Vector3(1, 0, 0);
	Vector3 y_base = Vector3(0, 1, 0);
	Vector3 z_base = Vector3(0, 0, 1);
	Ray x_ray = Ray(Vector3(0, 0, 0), Vector3(1, 0, 0));
	Ray y_ray = Ray(Vector3(0, 0, 0), Vector3(0, 1, 0));
	Ray z_ray = Ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
	/**
	 * a vector representing the translation of this basis relative to its parent. 
	 */
	void update_rays();
	Quat create_prioritized_rotation(Vector3 xHeading, Vector3 yHeading, Vector3 zHeading) {
		Quat toYZ = Quat(y_base * z_base * yHeading * zHeading);
		Vector3 tempV = toYZ.get_euler() * y_base;
		Quat toY = Quat(tempV * yHeading);
		return toY * toYZ;
	}
	void set(Vector3 p_x, Vector3 p_y, Vector3 p_z);

public:
	int LEFT = -1;
	int RIGHT = 1;
	int chirality = RIGHT;
	int NONE = -1;
	int X = 0;
	int Y = 1;
	int Z = 2;

	Quat rotation;
	Quat inverse_rotation;
	Vector3 translate;

	/**
	 * Initialize this basis at the origin. The basis will be right handed by default. 
	 * @param origin
	 */
	EWBIKTransform(Vector3 p_origin);

	EWBIKTransform();

	EWBIKTransform(const Transform &input);

	/**
	 * Initialize this basis at the origin.
	 *  The basis will be backed by a rotation object which presumes right handed chirality. 
	 *  Therefore, the rotation object will align so its local XY plane aligns with this basis' XY plane
	 *  Afterwards, it will check chirality, and if the basis isn't right handed, this class will assume the
	 *  z-axis is the one that's been flipped. 
	 *  
	 *  If you want to manually specify which axis has been flipped 
	 *  (so that the rotation object aligns with respect to the plane formed 
	 *  by the other two basis vectors) then use the constructor dedicated for that purpose
	 * @param origin
	 * @param x basis vector direction
	 * @param y basis vector direction
	 * @param z basis vector direction
	 */
	EWBIKTransform(Vector3 p_origin, Vector3 p_x, Vector3 p_y, Vector3 p_z);

	/**
	 * Initialize this basis at the origin defined by the base of the @param x Ray.
	 * 
	 *  The basis will be backed by a rotation object which presumes right handed chirality. 
	 *  Therefore, the rotation object will align so its local XY plane aligns with this basis' XY plane
	 *  Afterwards, it will check chirality, and if the basis isn't right handed, this class will assume the
	 *  z-axis is the one that's been flipped.
	 *   
	 *  If you want to manually specify which axis has been flipped 
	 *  (so that the rotation object aligns with respect to the plane formed 
	 *  by the other two basis vectors) then use the constructor dedicated for that purpose
	 *  
	 *  
	 * @param x basis Ray 
	 * @param y basis Ray 
	 * @param z basis Ray 
	 */
	EWBIKTransform(Ray p_x, Ray p_y, Ray p_z);

	/**
	 * takes on the same values as the input basis. 
	 * @param in
	 */
	void adopt_values(EWBIKTransform p_in);
	void set_identity();

	Quat get_local_of_rotation(Quat p_in_rot);

	void set_to_local_of(EWBIKTransform p_global_input, EWBIKTransform &r_local_output);

	void refresh_precomputed();

	Vector3 get_local_of(Vector3 p_v);

	void set_to_local_of(Vector3 p_input, Vector3 &r_output);

	void rotate_to(Quat p_new_rotation);

	void rotate_by(Quat p_add_rotation);
	/**
	 * sets globalOutput such that the result of 
	 * this->getLocalOf(globalOutput) == localInput. 
	 * 
	 * @param localInput
	 * @param globalOutput
	 */
	void set_to_global_of(EWBIKTransform localInput, EWBIKTransform &globalOutput);

	void set_to_global_of(Vector3 p_input, Vector3 &r_output);

	void translate_by(Vector3 p_translate_by);

	void translate_to(Vector3 p_new_origin);

	Ray get_x_ray();

	Ray get_y_ray();

	Ray get_z_ray();

	Vector3 get_x_heading();

	Vector3 get_y_heading();

	Vector3 get_z_heading();

	Vector3 get_origin();

	/**
	 * true if the input axis should be multiplied by negative one after rotation. 
	 * By default, this always returns false. But can be overridden for more advanced implementations
	 * allowing for reflection transformations. 
	 * @param axis
	 * @return true if axis should be flipped, false otherwise. Default is false. 
	 */
	bool is_axis_flipped(int axis);

	/**
	 * @return a precomputed inverse of the rotation represented by this basis object.
	 */
	Quat get_inverse_rotation();
};
