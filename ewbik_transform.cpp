#include "ewbik_transform.h"
#include "ewbik_transform.h"

Vector3 EWBIKTransform::get_local_of(Vector3 p_v) {
	Vector3 result = p_v;
	set_to_local_of(p_v, result);
	return result;
}

void EWBIKTransform::update_rays() {
	x_ray.position = this->translate;
	x_ray.normal = x_base;
	y_ray.position = this->translate;
	y_ray.normal = y_base;
	z_ray.position = this->translate;
	z_ray.normal = z_base;

	rotation.set_euler(x_ray.normal);
	rotation.set_euler(y_ray.normal);
	rotation.set_euler(z_ray.normal);

	x_ray.normal += this->translate;
	y_ray.normal += this->translate;
	z_ray.normal += this->translate;
}

void EWBIKTransform::set(Vector3 p_x, Vector3 p_y, Vector3 p_z) {
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
	this->rotation = create_prioritized_rotation(p_x, p_y, p_z);
	this->refresh_precomputed();
}

EWBIKTransform::EWBIKTransform(Ray p_x, Ray p_y, Ray p_z) {
	this->translate = p_x.position;
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

 EWBIKTransform::EWBIKTransform(Vector3 p_origin, Vector3 p_x, Vector3 p_y, Vector3 p_z) {
	this->translate = p_origin;
	x_ray = Ray(p_origin, p_origin);
	y_ray = Ray(p_origin, p_origin);
	z_ray = Ray(p_origin, p_origin);
	this->set(p_x, p_y, p_z);
}

 EWBIKTransform::EWBIKTransform(const Transform &input) {
	translate = input.origin;
	Vector3 x = input.basis.get_axis(0);
	Vector3 y = input.basis.get_axis(1);
	Vector3 z = input.basis.get_axis(2);
	set(x, y, z);
}

 EWBIKTransform::EWBIKTransform() {
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

 EWBIKTransform::EWBIKTransform(Vector3 p_origin) {
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

void EWBIKTransform::adopt_values(EWBIKTransform p_in) {
	this->translate = p_in.translate;
	this->rotation = p_in.rotation;
	x_base = translate * Vector3(1, 0, 0);
	y_base = translate * Vector3(0, 1, 0);
	z_base = translate * Vector3(0, 0, 1);
	x_ray = p_in.x_ray;
	y_ray = p_in.y_ray;
	z_ray = p_in.z_ray;
	this->refresh_precomputed();
}

void EWBIKTransform::set_identity() {
	this->translate = Vector3();
	x_base = Vector3(1, 0, 0);
	y_base = Vector3(0, 1, 0);
	z_base = Vector3(0, 0, 1);
	this->x_ray.position = this->translate;
	this->x_ray.normal = x_base;
	this->y_ray.position = this->translate;
	this->y_ray.normal = y_base;
	this->z_ray.position = this->translate;
	this->z_ray.normal = z_base;
	this->rotation = Quat();
	refresh_precomputed();
}

Quat EWBIKTransform::get_local_of_rotation(Quat p_in_rot) {
	Quat resultNew = inverse_rotation * p_in_rot * rotation;
	return resultNew;
}

void EWBIKTransform::set_to_local_of(Vector3 p_input, Vector3 &r_output) {
	r_output = p_input;
	r_output -= this->translate;
	r_output = inverse_rotation.get_euler() * r_output;
}

void EWBIKTransform::set_to_local_of(EWBIKTransform p_global_input, EWBIKTransform &r_local_output) {
	r_local_output.translate = this->get_local_of(p_global_input.translate);
	inverse_rotation *= p_global_input.rotation * r_local_output.rotation;
	r_local_output.refresh_precomputed();
}

void EWBIKTransform::refresh_precomputed() {
	this->rotation = inverse_rotation.inverse();
	this->update_rays();
}

void EWBIKTransform::rotate_to(Quat p_new_rotation) {
	this->rotation = p_new_rotation;
	this->refresh_precomputed();
}

void EWBIKTransform::rotate_by(Quat p_add_rotation) {
	this->rotation *= p_add_rotation;
	this->refresh_precomputed();
}

void EWBIKTransform::set_to_global_of(Vector3 p_input, Vector3 &r_output) {
	rotation = Quat(p_input);
	r_output += this->translate;
}

void EWBIKTransform::set_to_global_of(EWBIKTransform localInput, EWBIKTransform &globalOutput) {
	this->rotation *= localInput.rotation * globalOutput.rotation;
	this->set_to_global_of(localInput.translate, globalOutput.translate);
	globalOutput.refresh_precomputed();
}

void EWBIKTransform::translate_by(Vector3 p_translate_by) {
	this->translate.x += p_translate_by.x;
	this->translate.y += p_translate_by.y;
	this->translate.z += p_translate_by.z;
	update_rays();
}

void EWBIKTransform::translate_to(Vector3 p_new_origin) {
	this->translate.x = p_new_origin.x;
	this->translate.y = p_new_origin.y;
	this->translate.z = p_new_origin.z;
	update_rays();
}

Ray EWBIKTransform::get_x_ray() {
	return x_ray;
}

Ray EWBIKTransform::get_y_ray() {
	return y_ray;
}

Ray EWBIKTransform::get_z_ray() {
	return z_ray;
}

Vector3 EWBIKTransform::get_x_heading() {
	return this->x_ray.heading();
}

Vector3 EWBIKTransform::get_y_heading() {
	return this->y_ray.heading();
}

Vector3 EWBIKTransform::get_z_heading() {
	return this->z_ray.heading();
}

Vector3 EWBIKTransform::get_origin() {
	return translate;
}

bool EWBIKTransform::is_axis_flipped(int axis) {
	return false;
}

Quat EWBIKTransform::get_inverse_rotation() {
	return this->inverse_rotation;
}
