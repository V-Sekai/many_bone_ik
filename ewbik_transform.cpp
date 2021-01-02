#include "ewbik_transform.h"
#include "ewbik_transform.h"

Vector3 IKBasis::get_local_of(Vector3 p_v) {
	Vector3 result = p_v;
	set_to_local_of(p_v, result);
	return result;
}

void IKBasis::update_rays() {
	x_ray.position = translate;
	x_ray.normal = x_base;
	y_ray.position = translate;
	y_ray.normal = y_base;
	z_ray.position = translate;
	z_ray.normal = z_base;

	rotation.set_euler(x_ray.normal);
	rotation.set_euler(y_ray.normal);
	rotation.set_euler(z_ray.normal);

	x_ray.normal += translate;
	y_ray.normal += translate;
	z_ray.normal += translate;
}

void IKBasis::set(Vector3 p_x, Vector3 p_y, Vector3 p_z) {
	x_base = translate;
	y_base = translate;
	z_base = translate;
	x_base = Vector3(1, 0, 0);
	y_base = Vector3(0, 1, 0);
	z_base = Vector3(0, 0, 1);
	Vector3 zero;
	x_ray.position = zero;
	x_ray.normal = x_base;
	y_ray.position = zero;
	y_ray.normal = y_base;
	z_ray.position = zero;
	z_ray.normal = z_base;
	rotation = create_prioritized_rotation(p_x, p_y, p_z);
	refresh_precomputed();
}

IKBasis::IKBasis(Ray p_x, Ray p_y, Ray p_z) {
	translate = p_x.position;
	x_ray = p_x;
	y_ray = p_y;
	z_ray = p_z;
	Vector3 xDirNew = p_x.heading();
	Vector3 yDirNew = p_y.heading();
	Vector3 zDirNew = p_z.heading();
	xDirNew.normalize();
	yDirNew.normalize();
	zDirNew.normalize();
	set(xDirNew, yDirNew, zDirNew);
}

 IKBasis::IKBasis(Vector3 p_origin, Vector3 p_x, Vector3 p_y, Vector3 p_z) {
	translate = p_origin;
	x_ray = Ray(p_origin, p_origin);
	y_ray = Ray(p_origin, p_origin);
	z_ray = Ray(p_origin, p_origin);
	set(p_x, p_y, p_z);
}

 IKBasis::IKBasis() {
	Vector3 origin;
	translate = origin;
	x_base = origin;
	y_base = origin;
	z_base = origin;
	x_base *= Vector3(1, 0, 0);
	y_base *= Vector3(0, 1, 0);
	z_base *= Vector3(0, 0, 1);
	Vector3 zero;
	x_ray = Ray(zero, x_base);
	y_ray = Ray(zero, y_base);
	z_ray = Ray(zero, z_base);
	refresh_precomputed();
}

 IKBasis::IKBasis(Vector3 p_origin) {
	translate = p_origin;
	x_base = p_origin;
	y_base = p_origin;
	z_base = p_origin;
	x_base *= Vector3(1, 0, 0);
	y_base *= Vector3(0, 1, 0);
	z_base *= Vector3(0, 0, 1);
	Vector3 zero;
	x_ray = Ray(zero, x_base);
	y_ray = Ray(zero, y_base);
	z_ray = Ray(zero, z_base);
	refresh_precomputed();
}

void IKBasis::adopt_values(IKBasis p_in) {
	translate = p_in.translate;
	rotation = p_in.rotation;
	x_base = translate * Vector3(1, 0, 0);
	y_base = translate * Vector3(0, 1, 0);
	z_base = translate * Vector3(0, 0, 1);
	x_ray = p_in.x_ray;
	y_ray = p_in.y_ray;
	z_ray = p_in.z_ray;
	refresh_precomputed();
}

void IKBasis::set_identity() {
	translate = Vector3();
	x_base = Vector3(1, 0, 0);
	y_base = Vector3(0, 1, 0);
	z_base = Vector3(0, 0, 1);
	x_ray.position = translate;
	x_ray.normal = x_base;
	y_ray.position = translate;
	y_ray.normal = y_base;
	z_ray.position = translate;
	z_ray.normal = z_base;
	rotation = Quat();
	refresh_precomputed();
}

Quat IKBasis::get_local_of_rotation(Quat p_in_rot) {
	Quat resultNew = inverse_rotation * p_in_rot * rotation;
	return resultNew;
}

void IKBasis::set_to_local_of(Vector3 p_input, Vector3 &r_output) {
	r_output = p_input;
	r_output -= translate;
	r_output = inverse_rotation.get_euler() * r_output;
}

void IKBasis::set_to_local_of(IKBasis p_global_input, IKBasis &r_local_output) {
	r_local_output.translate = get_local_of(p_global_input.translate);
	inverse_rotation *= p_global_input.rotation * r_local_output.rotation;
	r_local_output.refresh_precomputed();
}

void IKBasis::refresh_precomputed() {
	rotation = inverse_rotation.inverse();
	update_rays();
}

void IKBasis::rotate_to(Quat p_new_rotation) {
	rotation = p_new_rotation;
	refresh_precomputed();
}

void IKBasis::rotate_by(Quat p_add_rotation) {
	rotation *= p_add_rotation;
	refresh_precomputed();
}

void IKBasis::set_to_global_of(Vector3 p_input, Vector3 &r_output) {
	rotation = Quat(p_input);
	r_output += translate;
}

void IKBasis::set_to_global_of(IKBasis localInput, IKBasis &globalOutput) {
	rotation *= localInput.rotation * globalOutput.rotation;
	set_to_global_of(localInput.translate, globalOutput.translate);
	globalOutput.refresh_precomputed();
}

void IKBasis::translate_by(Vector3 p_translate_by) {
	translate.x += p_translate_by.x;
	translate.y += p_translate_by.y;
	translate.z += p_translate_by.z;
	update_rays();
}

void IKBasis::translate_to(Vector3 p_new_origin) {
	translate.x = p_new_origin.x;
	translate.y = p_new_origin.y;
	translate.z = p_new_origin.z;
	update_rays();
}

Ray IKBasis::get_x_ray() {
	return x_ray;
}

Ray IKBasis::get_y_ray() {
	return y_ray;
}

Ray IKBasis::get_z_ray() {
	return z_ray;
}

Vector3 IKBasis::get_x_heading() {
	return x_ray.heading();
}

Vector3 IKBasis::get_y_heading() {
	return y_ray.heading();
}

Vector3 IKBasis::get_z_heading() {
	return z_ray.heading();
}

Vector3 IKBasis::get_origin() const {
	return translate;
}

bool IKBasis::is_axis_flipped(int axis) {
	return false;
}

Quat IKBasis::get_inverse_rotation() {
	return inverse_rotation;
}
Quat IKBasis::get_rotation() const {
	return rotation;
}